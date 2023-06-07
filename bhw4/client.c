#include "hash.h"
#include "liblibrary/book.h"

#include <errno.h>
#include <fts.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <arpa/inet.h>

#define PORT 8080

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int sendBook(Book *book, char *hostname) {
  struct sockaddr_in si_other;
  int s, i, slen = sizeof(si_other);

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    perror("socket");
    return 1;
  }

  memset((char *)&si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(PORT);

  if (inet_aton(hostname, &si_other.sin_addr) == 0) {
    perror("inet_aton() failed\n");
    return 1;
  }

  char buf[65536];
  buf[4] = 0;
  size_t wrote_bytes = serializeBook(book, buf + 5);

  crc32b(buf + 4, wrote_bytes + 1, buf);

  int wrote =
      sendto(s, buf, wrote_bytes + 5, 0, (struct sockaddr *)&si_other, slen);

  if (wrote != wrote_bytes + 5) {
    printf("Sendto fail\n");
    return 1;
  }

  unsigned int expected_response_crc;
  crc32b(buf, wrote_bytes + 5, &expected_response_crc);

  // 2 second timeout to recive an answer
  struct timeval read_timeout;
  read_timeout.tv_sec = 2;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

  int recv = recvfrom(s, buf, 65535, 0, (struct sockaddr *)&si_other, &slen);

  if (recv != 4) {
    printf("Recf fail\n");
    return 1;
  }

  if (expected_response_crc != *((unsigned int *)buf)) {
    printf("Recived unexpected CRC from server\n");
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: client hostname row\n");
    exit(1);
  }

  int row_number = atoi(argv[2]);

  FTS *ftsp;
  FTSENT *p, *chp;
  int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
  int rval = 0;
  char *paths[] = {argv[2], NULL};

  if ((ftsp = fts_open(paths, fts_options, NULL)) == NULL) {
    perror("fts_open");
    return -1;
  }

  /* Initialize ftsp with as many argv[] parts as possible. */
  chp = fts_children(ftsp, 0);
  if (chp == NULL) {
    return 0; /* no files to traverse */
  }

  char path_buf[2048] = {};
  memcpy(path_buf, argv[2], strlen(argv[2]));

  while ((p = fts_read(ftsp)) != NULL) {
    switch (p->fts_info) {
    case FTS_F:
      printf("Processing shelf %s\n", p->fts_path);

      FILE *f = fopen(p->fts_path, "rb");

      if (f == NULL) {
        perror("Can't open file!");
      }

      char str_buf[1024];
      int cur = 0;

      while (fgets(str_buf, sizeof(str_buf), f) != NULL) {
        sleep(1);

        if (str_buf[strlen(str_buf) - 1] == '\n') {
          str_buf[strlen(str_buf) - 1] = '\0';
        }

        Book *book =
            newBook(row_number, atoi(basename(p->fts_path)), cur, str_buf);

        printBook(book, stdout);

        int res;
        for (int i = 0; i < 5; ++i) {
          res = sendBook(book, argv[1]);

          if (res == 0) {
            break;
          }

          printf("Failed to send book. Retrying...\n");
        }

        if (res != 0) {
          printf("Failed to send message to server after 5 retries");
          return 1;
        }

        freeBook(book);

        cur += 1;
      }

      break;
    default:
      break;
    }
  }
  fts_close(ftsp);

  return 0;
}