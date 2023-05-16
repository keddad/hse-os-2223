#include "shared.h"

int main(int argc, char *argv[]) {
  if (argc != 5) {
    return 1;
  }

  char *lib_path = argv[1];
  int m = atoi(argv[2]);
  int n = atoi(argv[3]);
  int k = atoi(argv[4]);

  shared_data *shared_mem = create_shared_memory(sizeof(shared_data) * m, SHARED_MEM_NAME);
  sem_t** semaphores = malloc(sizeof(sem_t*) * m);

  for (int row = 0; row < m; row++) {
    char sem_name[4096];
    sprintf(sem_name, "/bhw2_7_%d", row);

    semaphores[row] = sem_open(sem_name, O_CREAT, 0644, 0);

    shared_mem[row].books = create_shared_memory(sizeof(char *) * k * n);

    for (int book = 0; book < k * n; book++) {
      shared_mem[row].books[book] =
          create_shared_memory(sizeof(char) * MAX_TITLE_SIZE);
    }

    // Initialize memory, do not start the actual workers
  }

  for (int row = 0; row < m; row++) {
    // "thread.join" but cooler
    sem_wait(semaphores[row]);
    sem_close(semaphores[row]);

    if (shared_mem[row].res != 0) {
      free(semaphores);
      return shared_mem[row].res;
    }
  }

  free(semaphores);

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