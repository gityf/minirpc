#ifndef _COMMON_IPC_SEMAPHORE_H
#define _COMMON_IPC_SEMAPHORE_H
#include <semaphore.h>
#include <cstring>
#include <errno.h>
#include <sys/time.h>
namespace wyf {

class Semaphore
{
public:
    explicit Semaphore(unsigned int value = 0) {
        if (sem_init(&mSem, 0, value) == -1) {
            throw(strerror(errno));
        };
    }

    void Wait() {
        int rtn;
        do {
            rtn = sem_wait(&mSem);
        } while ((rtn == -1) && (errno == EINTR));
        if (rtn == -1) {
            throw(strerror(errno));
        }
    }

    bool TryWait(unsigned int aMsec = 0) {
        if (aMsec == 0) {
            return (0 == sem_trywait(&mSem));
        } else {
            struct timeval tv = {};
            gettimeofday(&tv, NULL);

            tv.tv_sec  += aMsec / 1000;
            tv.tv_usec += (aMsec % 1000 * 1000);
            tv.tv_sec  += tv.tv_usec / 1000000;
            tv.tv_usec %= 1000000;

            struct timespec abstime = {};
            abstime.tv_sec  = tv.tv_sec;
            abstime.tv_nsec = tv.tv_usec * 1000;

            int ret = 0;
            do {
                ret = sem_timedwait(&mSem, &abstime);
            } while ((0 != ret) && (errno == EINTR));

            return (0 == ret);
        }
        return false;
    }

    void Post() {
        if (sem_post(&mSem) == -1) {
            throw(strerror(errno));
        }
    }
    
    int GetValue() {
        int val;
        if (sem_getvalue(&mSem, &val) == -1) {
            throw(strerror(errno));
        }
        return val;
    }

    ~Semaphore() {
        sem_destroy(&mSem);
    }
private:
    sem_t mSem;
};

} // namespace wyf
#endif /* _COMMON_IPC_SEMAPHORE_H */
