/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CShareMem.
*/
#ifndef _COMMON_IPC_CSHAREMEM_H_
#define _COMMON_IPC_CSHAREMEM_H_
#include <sys/sem.h>
namespace wyf {
#define RET_OK     0
#define RET_ERROR -1
#define SEM_LAST_PID  0
#define SEM_ZWAIT_CNT 1
#define SEM_NWAIT_CNT 2
#define SEM_REMOVE_FLAG 1
#define SHM_REMOVE_FLAG 2
#define SHM_DEPART_FLAG 4

typedef struct SShmBlock
{
    int iSemId;     /* Semaphore's IPC ID */
    int iLen;       /* Size of the iStr[] array */
    char iStr[1];   /* Start of variable storage */
} SShmBlock;

class CShareMem {
public:
    CShareMem();
    ~CShareMem();
    int InitData();

    // methods for semaphore.
    void SemOper(int aSemId, struct sembuf *aSemBuf);
    void SemLock(int aSemId);
    void SemUnlock(int aSemId);
    int  SemGet(int aKey, int aSemNum);
    void SemRm(int aSemId);
    int  SemStatus(int aSemId, int aCmd);

    // methods for share memory.
    int ShmGet(int aKey, int aShmSize);
    struct SShmBlock *ShmHeadGet(int aShmId);
    void ShmRm(int aShmId);
    void ShmDt(int aShmId);
    int  ShmInfos(int aShmId);
    void RemoveShm(int aKey);

private:
    // variables for semaphore.
    struct sembuf *iSemWait;
    struct sembuf *iSemNotify;
    int iSemId;

    // variables for share memory.
    struct SShmBlock *iShmBlock;
    int iShmId;
    unsigned int iClearFlag;
};

} // namespace wyf
#endif  // _COMMON_IPC_CSHAREMEM_H_
