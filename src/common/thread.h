#pragma once

#include "fastfetch.h"

#ifdef FF_HAVE_THREADS
    #if defined(_WIN32)
        #include <handleapi.h>
        #include <synchapi.h>
        #include <process.h>
        #include <processthreadsapi.h>
        #define FF_THREAD_MUTEX_INITIALIZER SRWLOCK_INIT
        typedef SRWLOCK FFThreadMutex;
        typedef HANDLE FFThreadType;
        static inline void ffThreadMutexLock(FFThreadMutex* mutex) { AcquireSRWLockExclusive(mutex); }
        static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { ReleaseSRWLockExclusive(mutex); }
        static inline FFThreadType ffThreadCreate(unsigned (__stdcall* func)(void*), void* data) {
            return (FFThreadType)_beginthreadex(NULL, 0, func, data, 0, NULL);
        }
        #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType) static __stdcall unsigned fn ## ThreadMain (void* data) { fn((paramType)data); return 0; }
        #define FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(fn) static __stdcall unsigned fn ## ThreadMain () { fn(); return 0; }
        static inline void ffThreadDetach(FFThreadType thread) { CloseHandle(thread); }
        static inline bool ffThreadJoin(FFThreadType thread, uint32_t timeout)
        {
            if (WaitForSingleObject(thread, timeout == 0 ? (DWORD) -1 : timeout) != 0 /*WAIT_OBJECT_0*/)
            {
                TerminateThread(thread, (DWORD) -1);
                CloseHandle(thread);
                return false;
            }
            CloseHandle(thread);
            return true;
        }
    #else
        #include <pthread.h>
        #include <signal.h>
        #if FF_HAVE_PTHREAD_NP
            #include <pthread_np.h>
        #endif
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
        #define FF_THREAD_ENTRY_DECL_WRAPPER_NOPARAM(fn) static void* fn ## ThreadMain () { fn(); return NULL; }
        static inline void ffThreadDetach(FFThreadType thread) { pthread_detach(thread); }
        static inline bool ffThreadJoin(FFThreadType thread, FF_MAYBE_UNUSED uint32_t timeout)
        {
            #if HAVE_TIMEDJOIN_NP
                if (timeout > 0)
                {
                    struct timespec ts;
                    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
                    {
                        ts.tv_sec += ts.tv_sec / 1000;
                        ts.tv_nsec += (ts.tv_nsec % 1000) * 1000000;
                        if (pthread_timedjoin_np(thread, NULL, &ts) != 0)
                        {
                            pthread_kill(thread, SIGTERM);
                            return false;
                        }
                        return true;
                    }
                }
            #endif
            pthread_join(thread, NULL);
            return true;
        }
    #endif
#else //FF_HAVE_THREADS
    #define FF_THREAD_MUTEX_INITIALIZER 0
    typedef char FFThreadMutex;
    static inline void ffThreadMutexLock(FFThreadMutex* mutex) { FF_UNUSED(mutex) }
    static inline void ffThreadMutexUnlock(FFThreadMutex* mutex) { FF_UNUSED(mutex) }
    #define FF_THREAD_ENTRY_DECL_WRAPPER(fn, paramType)
#endif //FF_HAVE_THREADS
