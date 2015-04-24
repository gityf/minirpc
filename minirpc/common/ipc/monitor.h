#ifndef _COMMON_IPC_MONITOR_H__
#define _COMMON_IPC_MONITOR_H__
#include "csemaphore.h"

class Monitor {
public:
    Monitor() {}

    ~Monitor() {}

    void TimeWait(unsigned int aMsec) {
        iSem.TryWait(aMsec);
    }

    void Wait() {
         iSem.Wait();
    }

    void Notify() {
        iSem.Post();
    }

private:
    Semaphore iSem;
};

#endif // _COMMON_IPC_MONITOR_H__
