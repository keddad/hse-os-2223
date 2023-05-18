#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "liblibrary/book.h"
#include "liblibrary/library.h"

#define PORT "8080"

int main(int argc, char *argv[]) {
  int sockfd; // some setup code liberated from Beej's guide to network
              // programming
  struct addrinfo hints, *servinfo, *p;
  socklen_t sin_size;
  int yes = 1;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, 256) == -1) {
    perror("listen");
    exit(1);
  }

  printf("Server is accepting connections\n");

  Library *library = newLibrary();

  while (1) {
    struct sockaddr_storage their_addr;
    sin_size = sizeof their_addr;

    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    char message_buffer[65536];

    int read_size =
        recv(new_fd, message_buffer, sizeof(message_buffer), MSG_WAITALL);

    if (read_size == -1) {
      perror("recv failed");
      continue;
    }

    if (read_size == 0) {
      perror("client disconnected");
      continue;
    }

    printf("Got a request of size %d\n", read_size);

    if (message_buffer[0] == 0) { // add book request
      Book *book = NULL;
      deserializeBook(&book, message_buffer + 1);

      printf("Got a new book! ");
      printBook(book, stdout);
      addBook(library, book);

      printLibrary(library, stdout);
      continue;
    }

    if (message_buffer[0] == 1) { // get view request
      perror("Not implemented");
    }

    close(new_fd);
  }

  return 0;
}