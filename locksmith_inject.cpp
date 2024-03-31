#include <stdio.h>
#include <funchook.h>
#include <stdexcept>
#include <pthread.h>

struct LockSmith_State {
    FILE* stream = nullptr;
};

static inline LockSmith_State locksmithState = {};

#define LocksmithLog(...) fprintf(locksmithState.stream, __VA_ARGS__); fflush(locksmithState.stream);

typedef FILE* (*fopen_orig)(const char *pathname, const char *mode);
typedef int (*fclose_orig)(FILE *stream);

typedef int (*pthread_mutex_lock_orig)(pthread_mutex_t *mutex);

static fopen_orig fopen_func = nullptr;
static fclose_orig fclose_func = nullptr;
static pthread_mutex_lock_orig pthread_mutex_lock_func = nullptr;

FILE* Locksmith_fopen(const char *pathname, const char *mode) {
    LocksmithLog("Locksmith_fopen( %s, %s ): fopen passthrough opened file\n", pathname, mode);

    return fopen_func(pathname, mode);
}

int Locksmith_MutexLock(pthread_mutex_t *mutex) {
    LocksmithLog("Locksmith_MutexLock( void ): Mutex locked\n");

    return pthread_mutex_lock_func(mutex);
}

void Locksmith_InstallHooks() {
    LocksmithLog("Locksmith_InstallHooks( void ): Installing hooks...\n");

    funchook_t* fopenHook = funchook_create();
    funchook_t* pthreadLockHook = funchook_create();

    fopen_func = fopen;
    fclose_func = fclose;
    pthread_mutex_lock_func = pthread_mutex_lock;

    // FIXME: Do error checking
    funchook_prepare(fopenHook, (void **) &fopen_func, (void *) Locksmith_fopen);
    funchook_prepare(pthreadLockHook, (void **) &pthread_mutex_lock_func, (void *) Locksmith_MutexLock);

    int i1 = funchook_install(fopenHook, 0);
    int i2 = funchook_install(pthreadLockHook, 0);


    if (i1 != 0 || i2 != 0) {
        throw std::runtime_error("Failed to install function hook(s)!");
    }
}

void Locksmith_InitState() {
    // make sure to init this before we install hooks!
    locksmithState.stream = fopen("locksmith.log", "a+");

    LocksmithLog("Locksmith_InitState( void ): Opened locksmith.log for appending\n");
}

// Loader

/**
 * Loads locksmith into the process by installing hooks and setting stuff up
 */
void __attribute__((constructor)) LockSmith_Bootstrap(void) {
    Locksmith_InitState();
    Locksmith_InstallHooks();
}
