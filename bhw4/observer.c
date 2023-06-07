#include "liblibrary/library.h"
#include "hash.h"

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

int reciveLibrary(Library **book, char *hostname) {
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
  buf[4] = 1;

  crc32b(buf + 4, 1, &buf);

  int wrote = sendto(s, buf, 5, 0, (struct sockaddr *)&si_other, slen);

  if (wrote != 5) {
    return 1;
  }

  // 2 second timeout to recive an answer
  struct timeval read_timeout;
  read_timeout.tv_sec = 2;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

  int recv = recvfrom(s, buf, 65535, 0, (struct sockaddr *)&si_other, &slen);

  if (recv < 4) {
    return 1;
  }

  unsigned int calculated_crc;
  crc32b(buf + 4, recv - 4, &calculated_crc);

  if (calculated_crc != *((unsigned int *)buf)) {
    printf("CRC is broken, discarding\n");
    return 1;
  }

  deserializeLibrary(book, buf + 4);

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: client hostname\n");
    exit(1);
  }

  Library *library = NULL;

  while (1) {
    sleep(3);

    int res;

    for (int i = 0; i < 5; ++i) {
      res = reciveLibrary(&library, argv[1]);

      if (res == 0) {
        break;
      }

      printf("Can't get info from server. Retrying...\n");
    }

    if (res != 0) {
      printf("Server is dead!\n");
      return 1;
    }

    printLibrary(library, stdout);
    freeLibrary(library);
  }

  return 0;
}