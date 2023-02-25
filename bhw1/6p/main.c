#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../punctuation.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Invalid argument number\n");
    return 1;
  }

  int readpipe[2], writepipe[2];
  char text_buf[10000] = {};
  int punct_buf[sizeof(puncts)] = {};

  pipe(readpipe);
  pipe(writepipe);

  pid_t is_io = fork();

  if (is_io) {
    printf("Reader process online\n");

    int read_fd = open(argv[1], O_RDONLY);
    int write_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);

    read(read_fd, text_buf, sizeof(text_buf));

    write(readpipe[1], text_buf, sizeof(text_buf));
    read(writepipe[0], punct_buf, sizeof(punct_buf));

    for (int i = 0; i < sizeof(puncts); ++i) {
      // It's obscure but it's actually POSIX
      dprintf(write_fd, "%c - %d\n", puncts[i], punct_buf[i]);
      printf("%c - %d\n", puncts[i], punct_buf[i]);
    }

    close(read_fd);
    close(write_fd);
  } else {

    printf("Processor process online\n");
    read(readpipe[0], text_buf, sizeof(text_buf));

    printf("Processor read string: %s\n", text_buf);

    count(text_buf, punct_buf);
    write(writepipe[1], punct_buf, sizeof(punct_buf));
  }

  close(readpipe[0]);
  close(readpipe[1]);
}