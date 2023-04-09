#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

int stop = 0;

void signal_handler() { stop = 1; }

void ensure_exists(char *file) {
  int fd = open(file, O_CREAT, S_IRWXU);

  if (fd == -1) {
    printf("Cannot create file: %s", file);
    exit(1);
  } else {
    close(fd);
  }
}

int main(int argc, char *argv[]) {
  if (access(MEMORY_KEY, F_OK) != -1 || access(SEMAPHORE_KEY_A, F_OK) != -1) {
    printf("%s or %s exist. Another instanse of server might be running\n",
           MEMORY_KEY, SEMAPHORE_KEY_A);
    return 1;
  }

  signal(SIGINT, signal_handler);
  signal(SIGSTOP, signal_handler);

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

  shared->shutdown = 0;
  shared->message = 0;

  printf("Server is waiting for client\n");

  while (1) {
    // Lock untill semaphore 0 reaches one
    // Put it to zero afterwards
    sem_wait(sem_a);

    if (shared->shutdown) {
      printf("Server shutdown\n");

      munmap(shared, sizeof(struct shared));
      close(fd_shm);
      shm_unlink(MEMORY_KEY);
      shm_unlink(SEMAPHORE_KEY_A);
      shm_unlink(SEMAPHORE_KEY_B);

      break;
    }

    if (stop) {
      shared->shutdown = 1;
    } else {
      printf("Server got message: %d\n", shared->message);
    }

    // Unlock client
    sem_post(sem_b);
  }

  return 0;
}