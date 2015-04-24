/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of daemon or others.
*/
#ifndef _COMMON_CDEAMON_H_
#define _COMMON_CDEAMON_H_
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
namespace wyf {
    // daemon mode.
    int DaemonMode() {
        const char *funcName = "DaemonMode";
        // fork to become!= group leader
        int pid;
        if ((pid = fork()) < 0) {
            fprintf(stderr, "%s::Cannot fork: %s.\n",
                funcName, strerror(errno));
            return -1;
        } else if (pid != 0) {
            // parent process => exit
            exit(0);
        }
        // become session leader to drop the ctrl. terminal
        if (setsid() < 0) {
            fprintf(stderr, "%s::setsid failed: %s.\n",
                funcName, strerror(errno));
        }
        // fork again to drop group  leadership
        if ((pid = fork()) < 0) {
            fprintf(stderr, "%s::Cannot fork: %s.\n",
                funcName, strerror(errno));
            return -1;
        } else if (pid != 0) {
            // parent process => exit
            exit(0);
        }

        // try to replace stdin, stdout & stderr with /dev/null
        if (freopen("/dev/null", "r", stdin) == 0) {
            fprintf(stderr, "%s::Cannot replace stdin with /dev/null: %s.\n",
                funcName, strerror(errno));
        }
        if (freopen("/dev/null", "w", stdout) == 0) {
            fprintf(stderr, "%s::Cannot replace stdout with /dev/null: %s.\n",
                funcName, strerror(errno));
        }
        if (freopen("/dev/null", "w", stderr) == 0) {
        fprintf(stderr, "%s::Cannot replace stderr with /dev/null: %s.\n",
        funcName, strerror(errno));
        }
        //*/
        return 0;
    }

    // file alone by pid file.
    void alone(const char* aPidFile) {
        int lf,val;
        char buf[10];
        if((lf=open(aPidFile,O_WRONLY|O_CREAT,0640)) < 0) {
            fprintf(stderr,"open pid file error\n");
            exit(1);
        }
        struct flock lock;
        lock.l_type=F_WRLCK;
        lock.l_start=0;
        lock.l_whence=SEEK_SET;
        lock.l_len=0;
        if (fcntl(lf,F_SETLK,&lock)<0) {
            if(errno==EACCES||errno ==EAGAIN){
                fprintf(stderr, "daemon is already running");
                fflush(stderr);
                exit(0);
            } else {
                fprintf(stderr,"fcntl F_SETLK error\n");
                fflush(stderr);
                exit(1);
            }
        }
        if (ftruncate(lf,0) < 0) {
            fprintf(stderr,"ftruncate error\n");
            fflush(stderr);
            exit(1);
        }
        sprintf(buf,"%d\n",getpid());
        if (write(lf,buf,strlen(buf)) != strlen(buf)) {
            fprintf(stderr,"write error\n");
            fflush(stderr);
            exit(1);
        }
        if ((val=fcntl(lf,F_GETFD,0)) < 0) {
            fprintf(stderr,"fcntl F_GETFD error\n");
            fflush(stderr);
            exit(1);
        }
        val |= FD_CLOEXEC;
        if ((val=fcntl(lf,F_SETFD,val)) < 0) {
            fprintf(stderr,"fcntl F_SETFD error\n");
            fflush(stderr);
            exit(1);
        }
    }
}
#endif  // _COMMON_CPEERFILTER_H_
