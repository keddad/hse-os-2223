#include "liblibrary/library.h"

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

int reciveLibrary(Library **book, char *hostname) {
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

  char buf[65536] = {};
  buf[0] = 1;

  size_t wrotten = write(sockfd, buf, 1);

  if (wrotten == -1) {
    perror("write ");
    return 1;
  }

  size_t read_size = recv(sockfd, buf, sizeof(buf), 0);

  if (read_size == -1) {
    perror("read ");
    return 1;
  }

  deserializeLibrary(book, buf);

  close(sockfd);
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

    int res = reciveLibrary(&library, argv[1]);

    if (res != 0) {
      printf("Server is dead!\n");
      return 1;
    }

    printLibrary(library, stdout);
    freeLibrary(library);
  }

  return 0;
}