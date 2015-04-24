/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Source file of class TinyThread.
*/

#include "common/tinythread.h"

#include <assert.h>
#include <stdlib.h>
#include <string>
#include "common/cuncopyable.h"
namespace {
void* ThreadFunc(void* ctx)
{
    ClosureA* func = reinterpret_cast< ClosureA*>(ctx);
    func->Run();
    return NULL;
}

} // anonymous namespace

class Thread : private CUncopyable
{
private:
    typedef ClosureA Callback;
    typedef pthread_t handle_type;
    Thread(Callback* func, const char* name);

    ~Thread();

    void Routine();

private:
    friend pthread_t CreateThread(Thread::Callback* func, const char *aName);

    std::string mName;
    Callback* mFunc;
    handle_type mHandle;
};

Thread::Thread(Thread::Callback* func, const char* name)
  : mFunc(func),
    mHandle(0)
{
    if (name != NULL) {
        mName = name;
    }
    Callback* routine = NewClosure(this, &Thread::Routine);
    if (::pthread_create(
            &mHandle, NULL, ThreadFunc, reinterpret_cast<void*>(routine))) {
        ::abort();
    }
}

Thread::~Thread()
{
}

void Thread::Routine()
{
    mFunc->Run();
    delete this;
}

ThreadHandle CreateThread(ClosureA* func, const char* name)
{
    Thread* thread = new Thread(func, name);
    assert(thread);
    Thread::handle_type handle = thread->mHandle;
    ::pthread_detach(handle);
    return handle;
}

