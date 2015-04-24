/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CShareMem.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common/ipc/csharemem.h"
namespace wyf {
CShareMem::CShareMem()
{
    iSemWait   = NULL;
    iSemNotify = NULL;
    iShmBlock  = NULL;
    iSemId = 0;
    iShmId = 0;
    iClearFlag = 0;
}

CShareMem::~CShareMem()
{
    if (iClearFlag & SEM_REMOVE_FLAG) {
        SemRm(iSemId);
    }
    if (iClearFlag & SHM_REMOVE_FLAG) {
        ShmRm(iShmId);
    }
    if (iClearFlag & SHM_DEPART_FLAG) {
        ShmDt(iShmId);
    }
    if (iSemWait != NULL) {
        free(iSemWait);
        iSemWait = NULL;
    }
    if (iSemNotify != NULL) {
        free(iSemNotify);
        iSemNotify = NULL;
    }
}

int CShareMem::InitData()
{
    iSemWait   = (struct sembuf *)malloc(sizeof(struct sembuf));
    iSemNotify = (struct sembuf *)malloc(sizeof(struct sembuf));
    if (iSemWait == NULL || iSemNotify == NULL) {
        return RET_ERROR;
    }

    // wait condition for lock is initialed.
    iSemWait->sem_num = 0;
    iSemWait->sem_op  = -1;
    iSemWait->sem_flg = SEM_UNDO;

    // notify condition for unlock is initialed.
    iSemNotify->sem_num = 0;
    iSemNotify->sem_op  = 1;
    iSemNotify->sem_flg = SEM_UNDO;
    return RET_OK;
}

void CShareMem::SemOper(int aSemId, struct sembuf *aSemBuf)
{
    int ret = 0;
    do {
        ret = semop(aSemId, aSemBuf, 1);

    } while (ret == -1 && errno == EINTR);
    if (ret) {
        fprintf(stderr, "SemOper:semop(2), ret:(%d),error:(%s)\n",
            ret, strerror(errno));
    }
}

void CShareMem::SemLock(int aSemId)
{
    SemOper(aSemId, iSemWait);
}

void CShareMem::SemUnlock(int aSemId)
{
    SemOper(aSemId, iSemNotify);
}

int CShareMem::SemGet(int aKey, int aSemNum)
{
    mode_t um, mode;
    int flags, semid = 0;
    um = umask(077);
    umask(um);
    mode = 0666 & ~um;
    flags = IPC_CREAT|IPC_EXCL;
    do {
        semid = semget(aKey, aSemNum, flags|mode);
        flags = IPC_CREAT;
    } while (semid == -1 && errno == EEXIST);
    if (semid == -1) {
        fprintf(stderr, "SemGet:semget(key:%d,num:%d),error:(%s)\n",
            aKey, aSemNum, strerror(errno));
        return RET_ERROR;
    }
    iSemId = semid;
    iClearFlag |= SEM_REMOVE_FLAG;
    return semid;
}

void CShareMem::SemRm(int aSemId)
{
    if(semctl(aSemId, 0, IPC_RMID) == -1)
        fprintf(stderr,"SemRm:semctl(semid=%d,IPC_RMID), error:(%s)\n",
        aSemId, strerror(errno));
    else
        fprintf(stderr, "SemRm:remove semid:%d ok.\n", aSemId);
    unsigned int f = SEM_REMOVE_FLAG;
    iClearFlag &= ~f;
}

int CShareMem::SemStatus(int aSemId, int aCmd)
{
    if (aCmd == SEM_LAST_PID) {
        return semctl(aSemId, 0, GETPID);
    } else if (aCmd == SEM_NWAIT_CNT) {
        return semctl(aSemId, 0, GETNCNT);
    } else if (aCmd == SEM_ZWAIT_CNT) {
        return semctl(aSemId, 0, GETZCNT);
    } else {
        return RET_ERROR;
    }
}

int CShareMem::ShmGet(int aKey, int aShmSize)
{
    mode_t um, mode;
    int flags, shmid = 0;
    um = umask(077);
    umask(um);
    mode = 0666 & ~um;
    flags = IPC_CREAT|IPC_EXCL;
    do {
        shmid = shmget(aKey, aShmSize+sizeof(struct SShmBlock), flags|mode);
        flags = IPC_CREAT;
    } while (shmid == -1 && errno == EEXIST);
    if (shmid == -1) {
        fprintf(stderr, "ShmGet:shmget(key:%d,size:%d),error:(%s)\n",
            aKey, aShmSize, strerror(errno));
        return RET_ERROR;
    }
    iShmBlock = (struct SShmBlock *)shmat(shmid, NULL, 0);
    if (iShmBlock == NULL) {
        fprintf(stderr, "ShmGet:shmat(key:%d,size:%d),error:(%s)\n",
            aKey, aShmSize, strerror(errno));
        return RET_ERROR;
    }
    iShmBlock->iLen = aShmSize;
    iShmBlock->iSemId = SemGet(aKey, 1);
    iClearFlag |= SHM_REMOVE_FLAG;
    iShmId = shmid;
    return shmid;
}

struct SShmBlock *CShareMem::ShmHeadGet(int aShmId)
{
    return iShmBlock;
}

void CShareMem::ShmRm(int aShmId)
{
    if(shmctl(aShmId, IPC_RMID, NULL) == -1)
        fprintf(stderr,"ShmRm:shmctl(shmid=%d), error:(%s)\n",
        aShmId, strerror(errno));
    else
        fprintf(stderr, "ShmRm:remove shmid:%d ok.\n", aShmId);
    unsigned int f = SHM_REMOVE_FLAG;
    iClearFlag &= ~f;
}

void CShareMem::ShmDt(int aShmId)
{
    if (iShmBlock == NULL) {
        return;
    }
    if(shmdt((void*)iShmBlock) == -1)
        fprintf(stderr,"ShmDel:shmdt(semid=%d), error:(%s)\n",
        iShmBlock->iSemId, strerror(errno));
    else
        fprintf(stderr, "ShmDel:depart shmid:%d ok.\n", iShmId);
    unsigned int f = SHM_DEPART_FLAG;
    iClearFlag &= ~f;
}

int CShareMem::ShmInfos(int aShmId)
{
    return 0;
}

void CShareMem::RemoveShm(int aKey)
{
    if (iShmBlock == NULL) {
        int shmid = ShmGet(aKey, 0);
        if (shmid >=0 && iShmBlock != NULL) {
            SemRm(iShmBlock->iSemId);
            ShmRm(shmid);
        }
    } else {
        SemRm(iShmBlock->iSemId);
        ShmRm(iShmId);
    }
}

} // namespace wyf
