/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of Thread ut.
*/
#include <iostream>
#include "common/tinythread.h"
#include "common/this_thread.h"
#include "gtest/gtest.h"
using namespace std;
static int sCount = 0;

void ThreadFunc()
{
    ++sCount;
}
void ThreadFunc1()
{
    int ii = 1000;
    while (--ii > 0) {
        ++sCount;
        cout << "Thread "
             << ThisThread::GetThreadId()
             << " is running Count = "
             << sCount
             << endl;
        ThisThread::SleepInUs(1000);
    }
}

struct Foo
{
    Foo() : mCount(0) {}
    void Bar()
    {
        ++mCount;
    }
    int mCount;
};

TEST(ThreadTest, LaunchWithFuncTest)
{
    Closure<void>* func = NewClosure(&ThreadFunc);
    ThreadHandle handle = CreateThread(func, "234");
    ::usleep(10000);
    EXPECT_NE(0u, handle);
    EXPECT_EQ(1, sCount);
}

TEST(ThreadTest, LaunchWithFuncTest2)
{
    for(int i = 0; i < 10; i++) {
  		Closure<void>* func = NewClosure(&ThreadFunc1);
  		ThreadHandle handle = CreateThread(func, "234");
  		::usleep(10000);
  		EXPECT_NE(0u, handle);
    }
    getchar();
}

TEST(ThreadTest, LaunchWithMethodTest)
{
    Foo foo;
    Closure<void>* func = NewClosure(&foo, &Foo::Bar);
    ThreadHandle handle = CreateThread(func, "123");
    ::usleep(10000);
    EXPECT_NE(0u, handle);
    EXPECT_EQ(1, foo.mCount);
}

