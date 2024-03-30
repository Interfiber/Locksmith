#include <stdio.h>
#define _GNU_SOURCE

void Locksmith_InstallHooks() {
    printf("Locksmith: Installing hooks...\n");
}

// Loader

/**
 * Loads locksmith into the process by installing hooks and setting stuff up
 */
void __attribute__((constructor)) LockSmith_Bootstrap() {
    printf("Locksmith: Locksmith_Bootstrap() called!\n");
    Locksmith_InstallHooks();
}
