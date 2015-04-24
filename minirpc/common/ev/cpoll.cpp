#include <sys/poll.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>

typedef struct SEventState {
    struct pollfd* ipfds;
    struct pollfd* iBakpfds;
    int ipfdsSize;
} SEventState;

int CEventPoll::ApiCreate(CEventLoop * aEventLoop)
{
    iEventState = reinterpret_cast<SEventState *>(malloc(sizeof(SEventState)));
    if (iEventState == NULL) {
        return RET_ERROR;
    }
    iEventLoop = aEventLoop;
    iEventState->ipfds = (struct pollfd *)malloc(iEventLoop->iSetSize * sizeof(struct pollfd));
    if (!iEventState->ipfds) {
        return RET_ERROR;
    }
    memset(iEventState->ipfds, 0, iEventLoop->iSetSize * sizeof(struct pollfd));
    iEventState->iBakpfds = (struct pollfd *)malloc(iEventLoop->iSetSize * sizeof(struct pollfd));
    if (!iEventState->iBakpfds) {
        return RET_ERROR;
    }
    memset(iEventState->iBakpfds, 0, iEventLoop->iSetSize * sizeof(struct pollfd));
    iEventState->ipfdsSize = 0;
    return RET_OK;
}

void CEventPoll::ApiFree()
{
    if (iEventState != NULL) {
        if (iEventState->ipfds != NULL) {
            free(iEventState->ipfds);
            iEventState->ipfds = NULL;
        }
        if (iEventState->iBakpfds != NULL) {
            free(iEventState->iBakpfds);
            iEventState->iBakpfds = NULL;
        }       
        free(iEventState);
        iEventState = NULL;
    }
}

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask)
{
    if (aSockId <= 0) return RET_ERROR;
    iEventState->ipfds[iEventState->ipfdsSize].fd      = aSockId;
    iEventState->ipfds[iEventState->ipfdsSize].events  = 0;
    iEventState->ipfds[iEventState->ipfdsSize].revents = 0;
    aAddMask |= iEventLoop->iFileEvent[aSockId].iMask; /* Merge old events */
    iEventLoop->iFileEvent[aSockId].iMask = aAddMask;
    
    if (aAddMask & AE_READABLE) {
        iEventState->ipfds[iEventState->ipfdsSize].events |= POLLIN;
    }

    if (aAddMask & AE_WRITABLE) {
        iEventState->ipfds[iEventState->ipfdsSize].events |= POLLOUT;
    }
    iEventState->ipfds[iEventState->ipfdsSize].events |= POLLERR | POLLNVAL | POLLHUP;
    iEventState->ipfdsSize++;
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask)
{
    int ii = 0;
    while (ii < iEventState->ipfdsSize) {
        if (iEventState->ipfds[ii].fd == aSockId) {
            iEventState->ipfdsSize--;
            iEventState->ipfds[ii] = iEventState->ipfds[iEventState->ipfdsSize];
            break;
        }
        ii++;
    }
}

int CEventPoll::ApiPollWait(struct timeval *aTvp)
{
    int retval, j, numevents = 0, fdsize = 0;
    fdsize = iEventState->ipfdsSize;
    memcpy(iEventState->iBakpfds, iEventState->ipfds, fdsize*sizeof(struct pollfd));

    retval = poll(iEventState->iBakpfds, fdsize, 
        aTvp ? (aTvp->tv_sec*1000 + aTvp->tv_usec/1000) : -1);

    if (retval > 0) {
        for (j = 0; j <= fdsize; j++) {
            int mask = 0;
            if (iEventState->iBakpfds[j].revents & POLLIN)  mask |= AE_READABLE;
            if (iEventState->iBakpfds[j].revents & POLLOUT) mask |= AE_WRITABLE;
            if (iEventState->iBakpfds[j].revents & POLLERR) mask |= AE_RWERROR;
            if (iEventState->iBakpfds[j].revents & POLLHUP) mask |= AE_WRITABLE;
            iEventLoop->iFiredEvent[numevents].iSockId = iEventState->iBakpfds[j].fd;
            iEventLoop->iFiredEvent[numevents].iMask   = mask;
            numevents++;
        }
    }
    return numevents;
}

const char *CEventPoll::ApiName() {
    return "poll";
}
