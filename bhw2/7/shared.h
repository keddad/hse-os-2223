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

#define SHARED_MEM_NAME "/posix-is-cool-books-are-too"
#define MEDADATA_INFO_NAME "/this-is-so-boring"

typedef struct {
  int res;
  char **books;
} shared_data;

typedef struct {
  int res;
  char **books;
} metadata;

void *create_shared_memory(size_t size, char* filename) {
  int fd_shm = shm_open(filename, O_RDWR | O_CREAT | O_EXCL, 0660);
  return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
              fd_shm, 0);
}
