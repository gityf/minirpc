/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CFileLock.
*/
#ifndef _COMMON_FILELOCK_H_
#define _COMMON_FILELOCK_H_
namespace wyf {
class CFileLock {
 public:
    CFileLock();
    ~CFileLock();
    int TryLockFd(int fd) {
        struct flock fl;
        memset(&fl, 0, sizeof(struct flock));
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        if (fcntl(fd, F_SETLK, &fl) == -1)  return RET_ERROR;
        return RET_OK;
    }

    int LockFd(int fd) {
        struct flock  fl;
        memset(&fl, 0, sizeof(struct flock));
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        if (fcntl(fd, F_SETLKW, &fl) == -1) return RET_ERROR;
        return RET_OK;
    }

    int UnLockFd(int fd) {
        struct flock  fl;
        memset(&fl, 0, sizeof(struct flock));
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_SET;
        if (fcntl(fd, F_SETLK, &fl) == -1) return RET_ERROR;
        return RET_OK;
    }

    int OpenLockFile(const char* aFile) {
        unlink(aFile);
        int fd = open(aFile, O_RDWR|O_CREAT, 0644);
        if (fd <= 0) return RET_ERROR;
        return fd;
    }
};
} // namespace wyf
#endif  // _COMMON_FILELOCK_H_
