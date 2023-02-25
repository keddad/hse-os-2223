#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../punctuation.h"

#define readpipe "/tmp/readpipe"
#define writepipe "/tmp/writepipe"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Invalid argument number\n");
    return 1;
  }

  char text_buf[10000] = {};
  int punct_buf[sizeof(puncts)] = {};

  mknod(readpipe, S_IFIFO | 0666, 0);
  mknod(writepipe, S_IFIFO | 0666, 0);

  pid_t is_io = fork();

  if (is_io) {
    printf("Reader process online\n");

    int read_fd = open(argv[1], O_RDONLY);
    int write_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);

    read(read_fd, text_buf, sizeof(text_buf));

    int read_pipe = open(readpipe, O_WRONLY);
    int write_pipe = open(writepipe, O_RDONLY);

    write(read_pipe, text_buf, sizeof(text_buf));
    read(write_pipe, punct_buf, sizeof(punct_buf));

    for (int i = 0; i < sizeof(puncts); ++i) {
      // It's obscure but it's actually POSIX
      dprintf(write_fd, "%c - %d\n", puncts[i], punct_buf[i]);
      printf("%c - %d\n", puncts[i], punct_buf[i]);
    }

    close(read_fd);
    close(write_fd);
    close(read_pipe);
    close(write_pipe);

    // This process is last to end, so unlink here
    unlink(readpipe);
    unlink(writepipe);
  } else {
    printf("Processor process online\n");

    int read_pipe = open(readpipe, O_RDONLY);
    int write_pipe = open(writepipe, O_WRONLY);

    read(read_pipe, text_buf, sizeof(text_buf));

    printf("Processor read string: %s\n", text_buf);

    count(text_buf, punct_buf);
    write(write_pipe, punct_buf, sizeof(punct_buf));
    close(read_pipe);
    close(write_pipe);
  }
}