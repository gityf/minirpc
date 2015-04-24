/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of method main.
*/

#include <csignal>
#include "agent/cagentservice.h"
#include "agent/chttphandler.h"
#include "agent/csessionrs.h"
#include "agent/csessionsync.h"
#include "agent/version.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/localdef.h"
#include "common/csingleton.h"
#include "common/cdaemon.h"
#include "common/csetproctitle.h"

void CatchSignal(void);
static bool gIsStoped;
// ---------------------------------------------------------------------------
// void OnSignalProc(int s)
//
// handle signal, if signal SIGTERM is received.
// the main process should be shutdown.
// ---------------------------------------------------------------------------
//
void OnSignalProc(int s) {
    const char *funcName = "OnSignalProc";
    LOG(LL_ERROR, "%s::Catch signal :%d", funcName, s);

    if (s == SIGUSR1) {
        LOG(LL_ERROR, "%s::signal :%d", funcName, s);
    } else {
        LOG(LL_ERROR, "%s::Catch signal :%d, exit!!!", funcName, s);
        gIsStoped = true;
        exit(0);
    }
    CatchSignal();
}

// ---------------------------------------------------------------------------
// void CatchSignal(void)
//
// catch signal, method OnSignalProc will handle the signals by default.
// ---------------------------------------------------------------------------
//
void CatchSignal(void) {
    DEBUG(LL_ALL, "CatchSignal::Begin.");
    signal(SIGABRT, OnSignalProc);  //22
    signal(SIGFPE, SIG_IGN);   //8
    signal(SIGTERM, OnSignalProc);  //15
    signal(SIGINT, OnSignalProc);  //2
    signal(SIGCLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);   //8
    signal(SIGALRM, SIG_IGN);
    DEBUG(LL_ALL, "CatchSignal::End.");
}

// ---------------------------------------------------------------------------
// void before_exit(void)
//
// before exit.
// ---------------------------------------------------------------------------
//
void before_exit(void) {
    LOG(LL_ERROR, "process %d exit.", getpid());
}

// ---------------------------------------------------------------------------
// int StartAgent()
//
// the entry of x_agent.
// ---------------------------------------------------------------------------
//
int StartAgent(string aConfigFile) {
    int ret = CSingleton<CConfig>::Instance()->InitConfig(aConfigFile);
    if (ret == RET_ERROR) {
        fprintf(stderr, "Error::Bad configuration!!!");
        return RET_ERROR;
    }
    if (CSingleton<CAgentService>::Instance()->InitData() == RET_ERROR) {
        fprintf(stderr, "Error::CAgentService.InitData failed!!!");
        return RET_ERROR;
    }
    if (CSingleton<CSessionRS>::Instance()->InitData() == RET_ERROR) {
        fprintf(stderr, "Error::CSessionRS.InitData failed!!!");
        return RET_ERROR;
    }
    if (CSingleton<CSessionSync>::Instance()->InitData() == RET_ERROR) {
        fprintf(stderr, "Error::CSessionSync.InitData failed!!!");
        return RET_ERROR;
    }
    CHttpHandler *httpHandler = new CHttpHandler();
    if (httpHandler == NULL || httpHandler->InitData() == RET_ERROR) {
        fprintf(stderr, "Error::CHttpHandler.InitData failed!!!");
        return RET_ERROR;
    }
    try {
        httpHandler->start();
    } catch (const string& catchStr) {
        fprintf(stderr, "Error::CHttpHandler.start failed:(%s)!!!",
            catchStr.c_str());
        return RET_ERROR;
    } catch (...) {
        return RET_ERROR;
    }
    try {
        CSingleton<CSessionRS>::Instance()->start();
    } catch (const string& catchStr) {
        fprintf(stderr, "Error::CSessionRS.start failed:(%s)!!!",
            catchStr.c_str());
        return RET_ERROR;
    } catch (...) {
        return RET_ERROR;
    }
    try {
        CSingleton<CSessionSync>::Instance()->start();
    } catch (const string& catchStr) {
        fprintf(stderr, "Error::CSessionSync.start failed:(%s)!!!",
            catchStr.c_str());
        return RET_ERROR;
    } catch (...) {
        return RET_ERROR;
    }
    gIsStoped = false;
    while (!gIsStoped) {
        // dump service informations every 20s.
        CSingleton<CAgentService>::Instance()->DumpAnyServiceInfos(time(NULL), MSG_TEXT_ALL);
        sleep(HB_TIME_OUT);
    }
    if (httpHandler != NULL) {
        delete httpHandler;
    }
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int main()
//
// the entry of x_agent.
// ---------------------------------------------------------------------------
//
int main(int argc, char* argv[]) {
    spt_init(argc, argv);
    int ch;
    bool isRunDeamon = false;

    string configFile("./agent.ini");

    while((ch = getopt(argc, argv, "c:l:dvh")) != -1) {
        switch(ch) {
        case 'c': {
                if (optarg) {
                    configFile = optarg;
                }
            }
            break;
        case 'd': {
                isRunDeamon = true;
            }
            break;
        case 'l': {
                if (optarg) {
                    int loglevel = atoi(optarg);
                    loglevel = loglevel == 0 ? 2 : loglevel;
                    CSingleton<CLogWriter>::Instance()->SetLogLevel(loglevel);
                }
            }
            break;
        default: {
                if (ch == 'v' || (optarg && strcmp(optarg, "-v") == 0)) {
                    fprintf(stderr, "%s version: %s, compile-time: %s\n",
                        argv[0], FullVersion, CompileInfo);
                } else if (ch == 'h' || (optarg && strcmp(optarg, "-h") == 0)) {
                    fprintf(stderr,"%s", HelpMsg);
                } else {
                    fprintf(stderr,"\tError:%s -h for help.\n", argv[0]);
                }
                fflush(stderr);
                fflush(stdout);
                _exit(0);
            }
            break;
        }
    }

    // run by daemon mode.
    if (isRunDeamon) {
        wyf::DaemonMode();
    }
    //register signal func
    atexit(before_exit);
    CatchSignal();
    setproctitle("%s:Master.%d", argv[0], getpid());
    do {
        int childPid = fork();
        if (childPid == 0) {
            setsid();
            atexit(before_exit);
            setproctitle("%s:%s", argv[0], "Worker.");
            CatchSignal();
            // child process.
            StartAgent(configFile);
            _exit(0);
        } else if (childPid > 0) {
            LOG(LL_ERROR, "Start agent worker process:(%lu).",
                childPid);
            int checkTimes = 3;
            do {
                sleep(1);
                if (kill(childPid, 0) == 0) {
                    // child exist...
                    checkTimes = 3;
                    sleep(5);
                }
            } while (checkTimes-- > 0);
            LOG(LL_ERROR, "agent worker process:(%lu) exit, try to start again.",
                childPid);
        }
        sleep(1);
    } while (1);

    return RET_OK;
}
// end of local file.
