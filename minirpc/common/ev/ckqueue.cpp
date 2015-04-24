#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef struct SEventState {
    int kqfd;
    struct kevent *events;
} SEventState;

int CEventPoll::ApiCreate(CEventLoop * aEventLoop)
{
    iEventState = (SEventState *)malloc(sizeof(SEventState));
    if (iEventState == NULL) {
        return RET_ERROR;
    }
    iEventLoop = aEventLoop;
    iEventState->kqfd = kqueue();
    if (iEventState->kqfd == RET_ERROR) {
        return RET_ERROR;
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

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask)
{
    if (aSockId <= 0) {
        return RET_ERROR;
    }
    aAddMask |= iEventLoop->iFileEvent[aSockId].iMask; /* Merge old events */
    iEventLoop->iFileEvent[aSockId].iMask = aAddMask;
    struct kevent ke;

    if (aAddMask & AE_READABLE) {
        EV_SET(&ke, aSockId, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(iEventState->kqfd, &ke, 1, NULL, 0, NULL) == RET_ERROR)
            return RET_ERROR;
    }
    if (aAddMask & AE_WRITABLE) {
        EV_SET(&ke, aSockId, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(iEventState->kqfd, &ke, 1, NULL, 0, NULL) == RET_ERROR)
            return RET_ERROR;
    }
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask)
{
    struct kevent ke;

    if (aDelMask & AE_READABLE) {
        EV_SET(&ke, aSockId, EVFILT_READ, EV_ADD, 0, 0, NULL);
    } else {
        EV_SET(&ke, aSockId, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    }
    kevent(iEventState->kqfd, &ke, 1, NULL, 0, NULL);
    if (aDelMask & AE_WRITABLE) {
        EV_SET(&ke, aSockId, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    } else {
        EV_SET(&ke, aSockId, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    }
    kevent(iEventState->kqfd, &ke, 1, NULL, 0, NULL);
}

int CEventPoll::ApiPollWait(struct timeval *aTvp)
{
    int retval, numevents = 0;

    if (aTvp != NULL) {
        struct timespec timeout;
        timeout.tv_sec = aTvp->tv_sec;
        timeout.tv_nsec = aTvp->tv_usec * 1000;
        retval = kevent(iEventState->kqfd, NULL, 0,
            iEventState->events, iEventLoop->iSetSize, &timeout);
    } else {
        retval = kevent(iEventState->kqfd, NULL, 0,
            iEventState->events, iEventLoop->iSetSize, NULL);
    }

    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct kevent *e = iEventState->events+j;

            if (e->filter == EVFILT_READ)  mask |= AE_READABLE;
            if (e->filter == EVFILT_WRITE) mask |= AE_WRITABLE;
            iEventLoop->iFiredEvent[j].iSockId = e->ident;
            iEventLoop->iFiredEvent[j].iMask   = mask;
        }
    }
    return numevents;
}

const char *CEventPoll::ApiName() {
    return "kqueue";
}
