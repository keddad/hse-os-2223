#include "shared.h"

int unpaid_worker(char *lib_path, shared_data *data, int row, int shelve_number,
                  int book_number) {
  shared_data *memory_object = &data[row];

  char sem_name[4096];
  sprintf(sem_name, "/bhw2_5_%d", row);

  sem_t* sem = sem_open(sem_name, 0);

  for (int shelve = 0; shelve < shelve_number; shelve++) {
    char path_buffer[4096] = {};

    // Bounds checking is for nerds
    // Cool kids create CVEs, not find them
    sprintf(path_buffer, "%s/%d_%d.txt", lib_path, row, shelve);
    printf("Processing %s\n", path_buffer);

    FILE *file = fopen(path_buffer, "r");

    if (file == NULL) {
      memory_object->res = 1;
      sem_post(sem);
      fclose(file);
      return 1;
    }

    for (int book = 0; book < book_number; book++) {
      char *res = fgets(memory_object->books[shelve * book_number + book],
                        MAX_TITLE_SIZE, file);

      if (res == NULL) {
        memory_object->res = 1;
        sem_post(sem);
        fclose(file);
        return 1;
      }
    }

    qsort(memory_object->books + (shelve * book_number), book_number, sizeof(char *), pstrcmp);
    fclose(file);
  }

  sem_post(sem);

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
}