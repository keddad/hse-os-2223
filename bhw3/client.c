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

#define PORT "8080"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int sendBook(Book *book, char *hostname) {
  int sockfd, numbytes;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
            sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure

  char buf[65536];
  buf[0] = 0;

  size_t wrote_bytes = serializeBook(book, buf + 1);
  write(sockfd, buf, wrote_bytes + 1);

  close(sockfd);
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

        sendBook(book, argv[1]);
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