/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class setproctitle.
*/
#ifndef _COMMON_CSET_PROCTITLE_H_
#define _COMMON_CSET_PROCTITLE_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus  */

void spt_init(int argc, char *argv[]);
void setproctitle(const char *fmt, ...);
const char* getproctitle();

#ifdef __cplusplus
}
#endif /* __cplusplus  */

#endif  // _COMMON_CSET_PROCTITLE_H_
