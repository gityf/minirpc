/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of this thread ut.
*/

#include "this_thread.h"
#include "gtest/gtest.h"

TEST(ThisThreadTest, BasicTest)
{
    int pthread_id = ThisThread::GetThreadId();
    int tid = ThisThread::GetId();
    ThisThread::Yield();
    ThisThread::SleepInMs(1);
    EXPECT_TRUE(pthread_id != tid);
}
