#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define MEMORY_KEY "/tmp/ihw6_shared_memory"
#define SEMAPHORE_KEY "/tmp/ihw6_semaphore"
#define PROJECT_ID 's'

union semun {
  int val;
  struct semid_ds *buf;
  ushort array[1];
};

struct shared {
    int shutdown;
    int message;
};

key_t get_key(char *file, char project) {
  key_t key;

  if ((key = ftok(file, project)) == -1) {
    perror("ftok");
    exit(1);
  }

  return key;
}