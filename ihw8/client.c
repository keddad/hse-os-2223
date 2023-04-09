#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "shared.h"

int stop = 0;

void signal_handler() { stop = 1; }

int main(int argc, char *argv[]) {
  signal(SIGINT, signal_handler);
  signal(SIGSTOP, signal_handler);

  srand(time(NULL));

  sem_t *sem_a, *sem_b;

  if ((sem_a = sem_open(SEMAPHORE_KEY_A, O_CREAT, 0660, 0)) == SEM_FAILED) {
    printf("Can't open Semaphore A\n");
    return 1;
  }

  if ((sem_b = sem_open(SEMAPHORE_KEY_B, O_CREAT, 0660, 1)) == SEM_FAILED) {
    printf("Can't open Semaphore B\n");
    return 1;
  }

  int fd_shm;

  if ((fd_shm = shm_open(MEMORY_KEY, O_RDWR | O_CREAT, 0660)) == -1) {
    printf("Can't get shared memory descriptor\n");
    return 1;
  }

  if (ftruncate(fd_shm, sizeof(struct shared)) == -1) {
    printf("Can't resize shared memory\n");
    return 1;
  }

  struct shared *shared;

  if ((shared = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd_shm, 0)) == MAP_FAILED) {
    perror("shmat");
    exit(1);
  }

  printf("Client is waitng for server.\n");

  int cur_val = 0;
  sem_getvalue(sem_b, &cur_val);

  // Workaround to weird deadlock on startup which happens ocasionally
  if (cur_val == 0) {
    sem_post(sem_b);
  }

  shared->shutdown = 0;
  shared->message = 0;

  while (1) {
    sem_wait(sem_b);

    sleep(1); // So we don't flood console

    if (!stop && !shared->shutdown) {
      int message = rand();
      printf("Sending message %d to server\n", message);
      shared->message = message;
    } else {
      printf("Shutting down\n");
      shared->shutdown = 1;

      // Unlock server
      sem_post(sem_a);

      munmap(shared, sizeof(struct shared));
      close(fd_shm);

      shm_unlink(MEMORY_KEY);
      shm_unlink(SEMAPHORE_KEY_A);
      shm_unlink(SEMAPHORE_KEY_B);

      break;
    }

    sem_post(sem_a);
  }

  return 0;
}