#pragma once

#ifndef FF_INCLUDED_common_thread
#define FF_INCLUDED_common_thread

#include "fastfetch.h"

#ifdef FF_HAVE_THREADS
    #if defined(_WIN32)
        #include <handleapi.h>
        #include <synchapi.h>
        #include <process.h>
        #define FF_THREAD_MUTEX_INITIALIZER SRWLOCK_INIT
        typedef SRWLOCK FFThreadMutex;
        typedef HANDLE FFThreadType;
        static inline void ffThreadMutexLock(FFThreadMutex* mutex) { AcquireSRWLockExclusive(mutex); }
        static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { ReleaseSRWLockExclusive(mutex); }
        static inline FFThreadType ffThreadCreate(unsigned (__stdcall* func)(void*), void* data) {
            return (FFThreadType)_beginthreadex(NULL, 0, func, data, 0, NULL);
        }
        #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType) static __stdcall unsigned fn ## ThreadMain (void* data) { fn((paramType)data); return 0; }
        static inline void ffThreadDetach(FFThreadType thread) { CloseHandle(thread); }
        static inline void ffThreadJoin(FFThreadType thread) { WaitForSingleObject(thread, 0xffffffff /*INFINITE*/); }
    #else
        #include <pthread.h>
        #define FF_THREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
        typedef pthread_mutex_t FFThreadMutex;
        typedef pthread_t FFThreadType;
        static inline void ffThreadMutexLock(FFThreadMutex* mutex) { pthread_mutex_lock(mutex); }
        static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { pthread_mutex_unlock(mutex); }
        static inline FFThreadType ffThreadCreate(void* (* func)(void*), void* data) {
            FFThreadType newThread = 0;
            pthread_create(&newThread, NULL, func, data);
            return newThread;
        }
        #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType) static void* fn ## ThreadMain (void* data) { fn((paramType)data); return NULL; }
        static inline void ffThreadDetach(FFThreadType thread) { pthread_detach(thread); }
        static inline void ffThreadJoin(FFThreadType thread) { pthread_join(thread, NULL); }
    #endif
#else //FF_HAVE_THREADS
    #define FF_THREAD_MUTEX_INITIALIZER 0
    typedef char FFThreadMutex;
    static inline void ffThreadMutexLock(FFThreadMutex* mutex) { FF_UNUSED(mutex) }
    static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { FF_UNUSED(mutex) }
    #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType)
#endif //FF_HAVE_THREADS

#endif
