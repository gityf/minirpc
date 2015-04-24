/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of atomic performance ut.
*/

#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include "atomic.h"
#include "common/this_thread.h"
#include "common/closure.h"
#include "common/tinythread.h"

class Bar
{
    struct Foo
    {
        int mValue;
        Foo* mNext;
    };
public:
    Bar() : mHead(0), mCount(0) {}
    int64_t GetCount()
    {
        return AtomicGet(&mCount);
    }
    void Producer(int count)
    {
        for (int k = 0; k < count; ++k) {
            Push(k);
        }
    }
    void Consumer(int count)
    {
        int num = 0;
        for (int k = 0; k < count; ) {
            if (Pop(&num)) {
                ++k;
                AtomicIncrement(&mCount);
            } else {
                pthread_yield();
            }
        }
    }
private:
    void Push(int v)
    {
        Foo* foo = new Foo;
        foo->mValue = v;
        Foo *head = 0;
        do {
            head = mHead;
            foo->mNext = head;
        } while (!AtomicCompareExchange(&mHead, foo, head));
    }
    bool Pop(int* v)
    {
        while (true) {
            Foo *head = mHead;
            if (!head) {
                return false;
            }
            Foo *next = head->mNext;
            if (!AtomicCompareExchange(&mHead, next, head)) {
                head->mNext = 0;
                *v = head->mValue;
                // memory leaked here
                // delete head;
                return true;
            }
        }
        return false;
    }

private:
    Foo* mHead;
    int64_t mCount;
};

int main(int argc, char** argv)
{
    int worker_count = 8;
    int target = 100000;
    if (argc > 1) {
        worker_count = atoi(argv[1]);
        worker_count = std::max(1, worker_count);
        worker_count = std::min(64, worker_count);
    }
    if (argc > 2) {
        target = atoi(argv[2]);
        target = std::max(1, target);
        target = std::min(100000000, target);
    }
    Bar bar;
    for (int k = 0; k < worker_count; ++k) {
        CreateThread(NewClosure(&bar, &Bar::Consumer, target));
    }
    for (int k = 0; k < worker_count; ++k) {
        CreateThread(NewClosure(&bar, &Bar::Producer, target));
    }

    while (true) {
        if (bar.GetCount() == target * worker_count) {
            break;
        }
        ThisThread::SleepInUs(100);
    }
    return 0;
}

