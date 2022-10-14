#pragma once

#ifndef FF_INCLUDED_common_thread
#define FF_INCLUDED_common_thread

#include "fastfetch.h"

#ifdef FF_HAVE_THREADS
    #if defined(_WIN32)
        #include <handleapi.h>
        #include <synchapi.h>
        #include <process.h> // Win32 <process.h> isn't available on MSYS2
        #define FF_THREAD_MUTEX_INITIALIZER SRWLOCK_INIT
        typedef SRWLOCK FFThreadMutex;
        static inline void ffThreadMutexLock(FFThreadMutex* mutex) { AcquireSRWLockExclusive(mutex); }
        static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { ReleaseSRWLockExclusive(mutex); }
        static inline void ffThreadCreateAndDetach(unsigned (__stdcall* func)(void*), void* data) {
            uintptr_t newThread = _beginthreadex(NULL, 0, func, data, 0, NULL);
            if(newThread)
                CloseHandle((HANDLE)newThread);
        }
        #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType) static __stdcall unsigned fn ## ThreadMain (void* data) { fn((paramType)data); return 0; }
    #else
        #include <pthread.h>
        #define FF_THREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
        typedef pthread_mutex_t FFThreadMutex;
        static inline void ffThreadMutexLock(FFThreadMutex* mutex) { pthread_mutex_lock(mutex); }
        static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { pthread_mutex_unlock(mutex); }
        static inline void ffThreadCreateAndDetach(void* (* func)(void*), void* data) {
            pthread_t newThread;
            if(pthread_create(&newThread, NULL, func, data) == 0)
                pthread_detach(newThread);
        }
        #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType) static void* fn ## ThreadMain (void* data) { fn((paramType)data); return NULL; }
    #endif
#else //FF_HAVE_THREADS
    #define FF_THREAD_MUTEX_INITIALIZER 0
    typedef char FFThreadMutex;
    static inline void ffThreadMutexLock(FFThreadMutex* mutex) { FF_UNUSED(mutex) }
    static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { FF_UNUSED(mutex) }
#endif //FF_HAVE_THREADS

#endif
