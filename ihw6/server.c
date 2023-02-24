#include <fcntl.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "shared.h"

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
  if (access(MEMORY_KEY, F_OK) != -1 || access(SEMAPHORE_KEY, F_OK) != -1) {
    printf("%s or %s exist. Another instanse of server might be running\n",
           MEMORY_KEY, SEMAPHORE_KEY);
    return 1;
  }

  ensure_exists(MEMORY_KEY);
  ensure_exists(SEMAPHORE_KEY);

  key_t mem_key = get_key(MEMORY_KEY, PROJECT_ID),
        sem_key = get_key(SEMAPHORE_KEY, PROJECT_ID);

  int sem_id, mem_id;

  if ((sem_id = semget(sem_key, 2, 0666 | IPC_CREAT)) == -1) {
    perror("semget");
    exit(1);
  }

  // We use semaphores as mutexes
  // 0 - Read semaphore (Server blocks on it untill it is 0)
  // 1 - Write semaphore (Client blocks on it untill it is 1)
  // This might be clumsy but it works (?)
  union semun arg;
  arg.val = 1;
  if (semctl(sem_id, 0, SETVAL, arg) == -1) {
    perror("semctl/0");
    exit(1);
  }

  arg.val = 0;
  if (semctl(sem_id, 1, SETVAL, arg) == -1) {
    perror("semctl/1");
    exit(1);
  }

  if ((mem_id = shmget(mem_key, sizeof(struct shared), 0660 | IPC_CREAT)) ==
      -1) {
    perror("shmget");
    exit(1);
  }

  struct shared *shared;

  if ((shared = (struct shared *)shmat(mem_id, NULL, 0)) ==
      (struct shared *)-1) {
    perror("shmat");
    exit(1);
  }

  shared->shutdown = 0;
  shared->message = 0;

  printf("Server is waiting for client\n");

  while (1) {
    struct sembuf sb = {0};

    // Lock untill semaphore 0 reaches zero
    if (semop(sem_id, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    }

    if (shared->shutdown) {
      printf("Server recived shutdown message\n");

      semctl(sem_id, 0, IPC_RMID);
      semctl(sem_id, 1, IPC_RMID);

      shmdt(shared);

      shmctl(mem_id, IPC_RMID, NULL);
      break;
    }

    printf("Server got message: %d\n", shared->message);

    // Lock itself
    sb.sem_op = 1;
    if (semop(sem_id, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    }

    // Unlock client
    sb.sem_num = 1;
    sb.sem_op = -1;

    if (semop(sem_id, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    }
  }

	unlink(MEMORY_KEY);
	unlink(SEMAPHORE_KEY);

  return 0;
}