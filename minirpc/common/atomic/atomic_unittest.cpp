/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of atomic ut.
*/

#include "atomic.h"
#include "gtest/gtest.h"

struct Foo
{
    int value;
    Foo* next;
};

struct Bar
{
    Bar() : foo(0) {}
    void Add(int value)
    {
        Foo *old;
        Foo *f = new Foo();
        f->value = value;
        do {
            old = foo;
            f->next = old;
        } while (!AtomicCompareExchange(&foo, f, old));
    }
    void Sub(int *value)
    {
        Foo *old;
        Foo *f;
        do {
            old = foo;
            f = foo->next;
        } while (!AtomicCompareExchange(&foo, f, old));
        AtomicSub(value, old->value);
    }
    Foo *foo;
};

TEST(AtomicUnitTest, IncrementTest)
{
    int n = 0;
    int m = AtomicIncrement(&n);
    EXPECT_EQ(m, 1);
    EXPECT_EQ(n, 1);
    AtomicAdd(&n, 3);
    EXPECT_EQ(n, 4);
}

TEST(AtomicUnitTest, ExchangeAddTest)
{
    int n = 5;
    EXPECT_EQ(AtomicExchangeAdd(&n, 4), 5);
    EXPECT_EQ(n, 9);
}

TEST(AtomicUnitTest, SetGetTest)
{
    int n = 0;
    AtomicSet(&n, 4);
    EXPECT_EQ(AtomicGet(&n), 4);
}

/*
TEST(AtomicUnitTest, CompareExchangeTest)
{
    stone::ThreadPool threadpool("test", 5);
    threadpool.Startup();
    int count = 50000;

    Bar bar;
    int sum = 0;
    for (int k = 0; k < count; ++k) {
        stone::ThreadPool::Closure *task =
            stone::NewClosure(&bar, &Bar::Add, k);
        threadpool.AddTask(task);
        sum += k;
    }
    threadpool.WaitForIdle();
    for (int k = 0; k < count; ++k) {
        stone::ThreadPool::Closure *task =
            stone::NewClosure(&bar, &Bar::Sub, &sum);
        threadpool.AddTask(task);
    }
    threadpool.WaitForIdle();
    EXPECT_EQ(sum, 0);

    threadpool.Shutdown();
}
*/
