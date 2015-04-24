/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Header file of atomic
*/

#ifndef _COMMON_ATOMIC_ATOMIC_H
#define _COMMON_ATOMIC_ATOMIC_H

#include "atomic_detail.h"

// compare the value stored in 'target' and 'compare', if they are equal,
// then set the 'exchange' into 'target' buffer.
// if the original value in '*target' equals to 'compare', return true
template<typename T>
inline bool AtomicCompareExchange(volatile T* target, T exchange, T compare)
{
    return AtomicDetail<sizeof(T)>::CompareExchange(
        target,
        exchange,
        compare);
}

// add 'value' to '*target', and return the original value.
template<typename T>
inline T AtomicExchangeAdd(volatile T* target, T value)
{
    return AtomicDetail<sizeof(T)>::ExchangeAdd(target, value);
}

// substract 'value' from '*target', and return the original value.
template<typename T>
inline T AtomicExchangeSub(volatile T* target, T value)
{
    return AtomicExchangeAdd(target, static_cast<T>(-value));
}

// add 'value' to '*target', and return the new value in 'target'.
template<typename T>
inline T AtomicAdd(volatile T* target, T value)
{
    return AtomicExchangeAdd(target, value) + value;
}

// substract 'value' from 'target', and return the new value in target.
template<typename T>
inline T AtomicSub(volatile T* target, T value)
{
    return AtomicExchangeSub(target, value) - value;
}

// set 'value' into 'target', and return the old value in target.
template<typename T>
inline T AtomicExchange(volatile T* target, T value)
{
    return AtomicDetail<sizeof(T)>::Exchange(target, value);
}

// set 'value' into '*target'
template<typename T>
inline void AtomicSet(volatile T* target, T value)
{
    AtomicDetail<sizeof(T)>::Set(target, value);
}

// get the value in '*target'
template<typename T>
inline T AtomicGet(volatile T* target)
{
    T value = *target;
    return value;
}

// add 1 to '*target', and return the new value in 'target'.
template<typename T>
inline T AtomicIncrement(volatile T* target)
{
    return AtomicAdd(target, static_cast<T>(1));
}

// substract 1 from '*target', and return the new value in 'target'.
template<typename T>
inline T AtomicDecrement(volatile T* target)
{
    return AtomicSub(target, static_cast<T>(1));
}

#endif // _COMMON_ATOMIC_ATOMIC_H

