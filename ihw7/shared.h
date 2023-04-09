#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define MEMORY_KEY "/ihw7_shared_memory"
#define SEMAPHORE_KEY_A "/ihw7_semaphore_a"
#define SEMAPHORE_KEY_B "/ihw7_semaphore_b"

struct shared {
    int shutdown;
    int message;
};