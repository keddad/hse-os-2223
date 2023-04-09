#include <fcntl.h>
#include <malloc.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_TITLE_SIZE 2048

typedef struct {
  sem_t sem;
  int res;
  char **books;
} shared_data;

int pstrcmp(const void *a, const void *b) {
  const char *rec1 = *(char **)a;
  const char *rec2 = *(char **)b;
  int val = strcmp(rec1, rec2);

  return val;
}

// God, save mmap
// This is the simplest way to create shared memory region
// It also clears itself once the master process (which created the memory
// region) is cleared
void *create_shared_memory(size_t size) {
  return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
              -1, 0);
}

int unpaid_worker(char *lib_path, shared_data *data, int row, int shelve_number,
                  int book_number) {
  shared_data *memory_object = &data[row];

  for (int shelve = 0; shelve < shelve_number; shelve++) {
    char path_buffer[4096] = {};

    // Bounds checking is for nerds
    // Cool kids create CVEs, not find them
    sprintf(path_buffer, "%s/%d_%d.txt", lib_path, row, shelve);
    printf("Processing %s\n", path_buffer);

    FILE *file = fopen(path_buffer, "r");

    if (file == NULL) {
      memory_object->res = 1;
      sem_post(&memory_object->sem);
      fclose(file);
      return 1;
    }

    for (int book = 0; book < book_number; book++) {
      char *res = fgets(memory_object->books[shelve * book_number + book],
                        MAX_TITLE_SIZE, file);

      if (res == NULL) {
        memory_object->res = 1;
        sem_post(&memory_object->sem);
        fclose(file);
        return 1;
      }
    }

    qsort(memory_object->books + (shelve * book_number), book_number, sizeof(char *), pstrcmp);
    fclose(file);
  }

  sem_post(&memory_object->sem);

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    return 1;
  }

  char *lib_path = argv[1];
  int m = atoi(argv[2]);
  int n = atoi(argv[3]);
  int k = atoi(argv[4]);

  shared_data *shared_mem = create_shared_memory(sizeof(shared_data) * m);

  for (int row = 0; row < m; row++) {
    sem_init(&shared_mem[row].sem, 1, 0);

    shared_mem[row].books = create_shared_memory(sizeof(char *) * k * n);

    for (int book = 0; book < k * n; book++) {
      shared_mem[row].books[book] =
          create_shared_memory(sizeof(char) * MAX_TITLE_SIZE);
    }

    pid_t pid = fork();

    if (pid == 0) {
      // We spawn a process for each shelf
      return unpaid_worker(lib_path, shared_mem, row, n, k);
    }
  }

  for (int row = 0; row < m; row++) {
    // "thread.join" but cooler
    sem_wait(&shared_mem[row].sem);
    sem_close(&shared_mem[row].sem);

    if (shared_mem[row].res != 0) {
      return shared_mem[row].res;
    }
  }

  int *cur_idx = calloc(m * n, sizeof(int));

  // Print reses in sorted order
  for (int book = 0; book < m * n * k; book++) {
    char *next = NULL;
    int m_taken = 0;
    int n_taken = 0;

    for (int row = 0; row < m; ++row) {
      for (int shelve = 0; shelve < n; shelve++) {
        if (cur_idx[row * m + shelve] == k) {
          continue;
        }

        if (next == NULL || strcmp(next, shared_mem[row].books[shelve * k + cur_idx[row * m + shelve]]) > 0) {
          next = shared_mem[row].books[shelve * k + cur_idx[row * m + shelve]];
          m_taken = row;
          n_taken = shelve;
        }
      }
    }

    printf("%s M: %d N: %d\n", next, m_taken, n_taken);

    cur_idx[m_taken * m + n_taken]++;
  }

  free(cur_idx);

  return 0;
}