#ifndef _COMMON_SINGLETONG_H_
#define _COMMON_SINGLETONG_H_

//
// singleton class
// Note: template class must 
//       provide default constructor.
//
#include <cstddef>
#include <cstdlib>
#include "common/cuncopyable.h"
template<class T>
class CSingleton : public CUncopyable {
public:
    // global access point
    static T *Instance() {
        if (iInstance == NULL) {
            // lock is needed in multi-thread
            iInstance = new T;
            ::atexit(CSingleton::Destroy);
        }
        return iInstance;
    }

    static T& InstanceRef() {
        if (iInstance == NULL) {
            iInstance = new T;
            ::atexit(CSingleton::Destroy);
        }
        return *iInstance;
    }

private:
    static void Destroy() {
        if (iInstance != NULL) {
            delete iInstance;
            iInstance = NULL;
        }
    }

private:
    //instance for T
    static T* volatile iInstance;
};
template<typename T>
T* volatile CSingleton<T>::iInstance = NULL;
#endif // _COMMON_SINGLETONG_H_
