#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "shared.h"

int stop = 0;

void signal_handler() { stop = 1; }

int main(int argc, char *argv[]) {
  if (access(MEMORY_KEY, F_OK) == -1 || access(SEMAPHORE_KEY, F_OK) == -1) {
    printf("%s or %s don't exist. Is server running?\n", MEMORY_KEY,
           SEMAPHORE_KEY);
    return 1;
  }

  signal(SIGINT, signal_handler);
  signal(SIGSTOP, signal_handler);

  srand(time(NULL));

  key_t mem_key = get_key(MEMORY_KEY, PROJECT_ID),
        sem_key = get_key(SEMAPHORE_KEY, PROJECT_ID);

  int sem_id, mem_id;

  if ((sem_id = semget(sem_key, 2, 0666)) == -1) {
    perror("semget");
    exit(1);
  }

  if ((mem_id = shmget(mem_key, sizeof(struct shared), 0660)) == -1) {
    perror("shmget");
    exit(1);
  }

  struct shared *shared;

  if ((shared = (struct shared *)shmat(mem_id, NULL, 0)) ==
      (struct shared *)-1) {
    perror("shmat");
    exit(1);
  }

  printf("Client is waitng for server.\n");

  while (1) {
    struct sembuf sb = {0};
    sb.sem_num = 1;

    // Lock untill semaphore 1 reaches zero
    if (semop(sem_id, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    }

    sleep(1); // So we don't flood console

    if (!stop) {
      int message = rand();
      printf("Sending message %d to server\n", message);
      shared->message = message;
    } else {
      printf("Shutting down\n");
      shared->shutdown = 1;

      // Unlock server
      sb.sem_num = 0;
      sb.sem_op = -1;

      if (semop(sem_id, &sb, 1) == -1) {
        perror("semop");
        exit(1);
      }

      shmdt(shared);
      break;
    }

    // Lock itself
    sb.sem_op = 1;
    if (semop(sem_id, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    }

    // Unlock server
    sb.sem_num = 0;
    sb.sem_op = -1;

    if (semop(sem_id, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    }
  }

  return 0;
}