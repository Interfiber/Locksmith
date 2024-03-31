#include <stdio.h>
#include <funchook.h>
#include <stdexcept>
#include <pthread.h>
#include <sys/file.h>
#include <unistd.h>

struct LockSmith_State {
    FILE* stream = nullptr;
};

static inline LockSmith_State locksmithState = {};

#define LocksmithLog(...) fprintf(locksmithState.stream, __VA_ARGS__); fflush(locksmithState.stream);

typedef FILE* (*fopen_orig)(const char *pathname, const char *mode);
typedef int (*fclose_orig)(FILE *stream);

typedef int (*pthread_mutex_lock_orig)(pthread_mutex_t *mutex);
typedef int (*pthread_mutex_unlock_orig)(pthread_mutex_t *mutex);

static fopen_orig fopen_func = nullptr;
static fclose_orig fclose_func = nullptr;
static pthread_mutex_lock_orig pthread_mutex_lock_func = nullptr;
static pthread_mutex_unlock_orig pthread_mutex_unlock_func = nullptr;

FILE* Locksmith_fopen(const char *pathname, const char *mode) {
    LocksmithLog("Locksmith_fopen( %s, %s ): fopen passthrough opened file\n", pathname, mode);

    return fopen_func(pathname, mode);
}

int Locksmith_MutexUnlock(pthread_mutex_t *mutex) {
    int res = pthread_mutex_unlock_func(mutex);

    unsigned int threadID = mutex->__data.__owner;
    LocksmithLog("Locksmith_MutexUnlock( %i ): Mutex unlocked, owned by: %i\n", getpid(), threadID);


    return res;
}

int Locksmith_MutexLock(pthread_mutex_t *mutex) {
    int res = pthread_mutex_lock_func(mutex);

    unsigned int threadID = mutex->__data.__owner;
    LocksmithLog("Locksmith_MutexLock( %i ): Mutex locked, owned by: %i\n", getpid(), threadID);

    return res;
}

void Locksmith_InstallHooks() {
    LocksmithLog("Locksmith_InstallHooks( %i ): Installing hooks...\n", getpid());

    funchook_t* fopenHook = funchook_create();
    funchook_t* pthreadLockHook = funchook_create();
    funchook_t* pthreadUnlockHook = funchook_create();

    fopen_func = fopen;
    fclose_func = fclose;
    pthread_mutex_lock_func = pthread_mutex_lock;
    pthread_mutex_unlock_func = pthread_mutex_unlock;

    // FIXME: Do error checking
    funchook_prepare(fopenHook, (void **) &fopen_func, (void *) Locksmith_fopen);
    funchook_prepare(pthreadLockHook, (void **) &pthread_mutex_lock_func, (void *) Locksmith_MutexLock);
    funchook_prepare(pthreadUnlockHook, (void **) &pthread_mutex_unlock_func, (void *) Locksmith_MutexUnlock);

    int i1 = funchook_install(fopenHook, 0);
    int i2 = funchook_install(pthreadLockHook, 0);
    int i3 = funchook_install(pthreadUnlockHook, 0);


    if (i1 != 0 || i2 != 0 || i3 != 0) {
        throw std::runtime_error("Failed to install function hook(s)!");
    }
}

int Locksmith_InitState() {
    // make sure to init this before we install hooks!
    locksmithState.stream = fopen("locksmith.log", "a+");

    LocksmithLog("Locksmith_InitState( void ): Opened locksmith.log for appending\n");

    return 0;
}

// Loader

/**
 * Loads locksmith into the process by installing hooks and setting stuff up
 */
void __attribute__((constructor)) LockSmith_Bootstrap(void) {
    if (Locksmith_InitState() == -1) return;
    Locksmith_InstallHooks();
}

void __attribute__((destructor)) LockSmith_Shutdown(void) {
    fclose(locksmithState.stream);
}
