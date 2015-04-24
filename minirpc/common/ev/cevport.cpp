#include <assert.h>
#include <errno.h>
#include <port.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>

static int evport_debug = 0;

/*
 * This file implements the ae API using event ports, present on Solaris-based
 * systems since Solaris 10.  Using the event port interface, we associate file
 * descriptors with the port.  Each association also includes the set of poll(2)
 * events that the consumer is interested in (e.g., POLLIN and POLLOUT).
 *
 * There's one tricky piece to this implementation: when we return events via
 * aeApiPoll, the corresponding file descriptors become dissociated from the
 * port.  This is necessary because poll events are level-triggered, so if the
 * fd didn't become dissociated, it would immediately fire another event since
 * the underlying state hasn't changed yet.  We must re-associate the file
 * descriptor, but only after we know that our caller has actually read from it.
 * The ae API does not tell us exactly when that happens, but we do know that
 * it must happen by the time aeApiPoll is called again.  Our solution is to
 * keep track of the last fds returned by aeApiPoll and re-associate them next
 * time aeApiPoll is invoked.
 *
 * To summarize, in this module, each fd association is EITHER (a) represented
 * only via the in-kernel association OR (b) represented by pending_fds and
 * pending_masks.  (b) is only true for the last fds we returned from aeApiPoll,
 * and only until we enter aeApiPoll again (at which point we restore the
 * in-kernel association).
 */
#define MAX_EVENT_BATCHSZ 512

typedef struct SEventState {
    int     portfd;                             /* event port */
    int     npending;                           /* # of pending fds */
    int     pending_fds[MAX_EVENT_BATCHSZ];     /* pending fds */
    int     pending_masks[MAX_EVENT_BATCHSZ];   /* pending fds' masks */
} SEventState;

int CEventPoll::ApiCreate(CEventLoop * aEventLoop)
{
    iEventState = (SEventState *)malloc(sizeof(SEventState));
    if (iEventState == NULL) {
        return RET_ERROR;
    }
    iEventLoop = aEventLoop;

    iEventState->portfd = port_create();
    if (iEventState->portfd == RET_ERROR) {
        return RET_ERROR;
    }

    iEventState->npending = 0;

    int i = 0;
    for (i = 0; i < MAX_EVENT_BATCHSZ; i++) {
        iEventState->pending_fds[i]   = -1;
        iEventState->pending_masks[i] = AE_NONE;
    }
    return RET_OK;
}

void CEventPoll::ApiFree()
{
    if (iEventState != NULL) {
        free(iEventState);
        iEventState = NULL;
    }
}

static int ApiLookupPending(SEventState *state, int fd) {
    int i;

    for (i = 0; i < state->npending; i++) {
        if (state->pending_fds[i] == fd)
            return (i);
    }

    return (-1);
}

/*
 * Helper function to invoke port_associate for the given fd and mask.
 */
static int ApiAssociate(const char *where, int portfd, int fd, int mask) {
    int events = 0;
    int rv, err;
    // error event.
    events = POLLERR | POLLHUP | POLLNVAL;
    if (mask & AE_READABLE) events |= POLLIN;
    if (mask & AE_WRITABLE) events |= POLLOUT;

    if (evport_debug)
        fprintf(stderr, "%s: port_associate(%d, 0x%x) = ", where, fd, events);

    rv = port_associate(portfd, PORT_SOURCE_FD, fd, events,
        (void *)(uintptr_t)mask);
    err = errno;

    if (evport_debug)
        fprintf(stderr, "%d (%s)\n", rv, rv == 0 ? "no error" : strerror(err));

    if (rv == -1) {
        fprintf(stderr, "%s: port_associate: %s\n", where, strerror(err));

        if (err == EAGAIN)
            fprintf(stderr, "ApiAssociate: event port limit exceeded.");
    }

    return rv;
}

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask)
{
    int fullmask, pfd;
    if (evport_debug)
        fprintf(stderr, "ApiAddEvent: fd %d mask 0x%x\n", aSockId, aAddMask);

    /*
     * Since port_associate's "events" argument replaces any existing events, we
     * must be sure to include whatever events are already associated when
     * we call port_associate() again.
     */
    fullmask = aAddMask | iEventLoop->iFileEvent[aSockId].iMask;
    pfd = ApiLookupPending(iEventState, aSockId);

    if (pfd != RET_ERROR) {
        /*
         * This fd was recently returned from aeApiPoll.  It should be safe to
         * assume that the consumer has processed that poll event, but we play
         * it safer by simply updating pending_mask.  The fd will be
         * re-associated as usual when aeApiPoll is called again.
         */
        if (evport_debug)
            fprintf(stderr, "aeApiAddEvent: adding to pending fd %d\n", aSockId);
        iEventState->pending_masks[pfd] |= fullmask;
        return RET_OK;
    }

    return (ApiAssociate("ApiAddEvent", iEventState->portfd, aSockId, fullmask));
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask)
{
    int fullmask, pfd;

    if (evport_debug)
        fprintf(stderr, "del fd %d mask 0x%x\n", aSockId, aDelMask);

    pfd = ApiLookupPending(iEventState, aSockId);

    if (pfd != RET_ERROR) {
        if (evport_debug)
            fprintf(stderr, "deleting event from pending fd %d\n", aSockId);

        /*
         * This fd was just returned from aeApiPoll, so it's not currently
         * associated with the port.  All we need to do is update
         * pending_mask appropriately.
         */
        iEventState->pending_masks[pfd] &= ~aDelMask;

        if (iEventState->pending_masks[pfd] == AE_NONE)
            iEventState->pending_fds[pfd] = -1;

        return;
    }

    /*
     * The fd is currently associated with the port.  Like with the add case
     * above, we must look at the full mask for the file descriptor before
     * updating that association.  We don't have a good way of knowing what the
     * events are without looking into the eventLoop state directly.  We rely on
     * the fact that our caller has already updated the mask in the eventLoop.
     */

    fullmask = iEventLoop->iFileEvent[aSockId].iMask;
    if (fullmask == AE_NONE) {
        /*
         * We're removing *all* events, so use port_dissociate to remove the
         * association completely.  Failure here indicates a bug.
         */
        if (evport_debug)
            fprintf(stderr, "ApiDelEvent: port_dissociate(%d)\n", aSockId);

        if (port_dissociate(iEventState->portfd, PORT_SOURCE_FD, aSockId) != 0) {
            perror("ApiDelEvent: port_dissociate");
            abort(); /* will not return */
        }
    } else if (ApiAssociate("ApiDelEvent", iEventState->portfd, aSockId,
        fullmask) != 0) {
        /*
         * ENOMEM is a potentially transient condition, but the kernel won't
         * generally return it unless things are really bad.  EAGAIN indicates
         * we've reached an resource limit, for which it doesn't make sense to
         * retry (counter-intuitively).  All other errors indicate a bug.  In any
         * of these cases, the best we can do is to abort.
         */
        abort(); /* will not return */
    }
}

int CEventPoll::ApiPollWait(struct timeval *aTvp)
{
    struct timespec timeout, *tsp;
    int mask, i;
    uint_t nevents;
    port_event_t event[MAX_EVENT_BATCHSZ];

    /*
     * If we've returned fd events before, we must re-associate them with the
     * port now, before calling port_get().  See the block comment at the top of
     * this file for an explanation of why.
     */
    for (i = 0; i < iEventState->npending; i++) {
        if (iEventState->pending_fds[i] == -1)
            /* This fd has since been deleted. */
            continue;

        if (ApiAssociate("ApiPollWait", iEventState->portfd,
            iEventState->pending_fds[i], iEventState->pending_masks[i]) != 0) {
            /* See aeApiDelEvent for why this case is fatal. */
            abort();
        }

        iEventState->pending_masks[i] = AE_NONE;
        iEventState->pending_fds[i] = -1;
    }

    iEventState->npending = 0;

    if (aTvp != NULL) {
        timeout.tv_sec = aTvp->tv_sec;
        timeout.tv_nsec = aTvp->tv_usec * 1000;
        tsp = &timeout;
    } else {
        tsp = NULL;
    }

    /*
     * port_getn can return with errno == ETIME having returned some events (!).
     * So if we get ETIME, we check nevents, too.
     */
    nevents = 1;
    if (port_getn(iEventState->portfd, event, MAX_EVENT_BATCHSZ, &nevents,
        tsp) == -1 && (errno != ETIME || nevents == 0)) {
        if (errno == ETIME || errno == EINTR)
            return RET_OK;

        /* Any other error indicates a bug. */
        perror("ApiPollWait: port_get");
        abort();
    }

    iEventState->npending = nevents;

    for (i = 0; i < nevents; i++) {
            mask = 0;
            if (event[i].portev_events & POLLIN)
                mask |= AE_READABLE;
            if (event[i].portev_events & POLLOUT)
                mask |= AE_WRITABLE;
			if (event[i].portev_events & (POLLERR | POLLHUP | POLLNVAL))
                mask |= AE_RWERROR;

            iEventLoop->iFiredEvent[i].iSockId = event[i].portev_object;
            iEventLoop->iFiredEvent[i].iMask = mask;

            if (evport_debug)
                fprintf(stderr, "ApiPollWait: fd %d mask 0x%x\n",
                    (int)event[i].portev_object, mask);

            iEventState->pending_fds[i] = event[i].portev_object;
            iEventState->pending_masks[i] = (uintptr_t)event[i].portev_user;
    }

    return nevents;
}

const char *CEventPoll::ApiName() {
    return "evport";
}
