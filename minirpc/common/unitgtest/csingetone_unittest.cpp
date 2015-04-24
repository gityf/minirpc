
#include "common/csingleton.h"

#include <iostream>
#include "gtest/gtest.h"

class Foo
{
public:
    Foo() : mCount(0) {}
    ~Foo()
    {
        // following code is just to verify the dtor of the singleton object
        // has been called. disable it when checkin code.
        // std::cout << "Foo::~Foo()" << std::endl;
    }
    void Increment()
    {
        ++mCount;
    }
    void Decrement()
    {
        --mCount;
    }
    int Get()
    {
        return mCount;
    }
    void Reset()
    {
        mCount = 0;
    }

private:
    int mCount;
};

TEST(SingletonTest, BasicTest)
{
    {
        Foo& foo = CSingleton<Foo>::InstanceRef();
        foo.Increment();
        foo.Increment();
    }
    // now verify the the singleton object from another scope
    {
        Foo& foo = CSingleton<Foo>::InstanceRef();
        EXPECT_EQ(2, foo.Get());
    }
    {
        Foo *foo = CSingleton<Foo>::Instance();
        foo->Increment();
        foo->Increment();
    }
    {
        Foo& foo = CSingleton<Foo>::InstanceRef();
        EXPECT_EQ(4, foo.Get());
    }
    {
        Foo *foo = CSingleton<Foo>::Instance();
        EXPECT_EQ(4, foo->Get());
    }
}
