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

#define PORT 8080

int main(int argc, char *argv[]) {
  struct sockaddr_in si_me, si_other;
	
	int s, i, slen = sizeof(si_other) , recv_len;
	
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return 1;
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		return 1;
	}

  printf("Server is accepting connections\n");

  Library *library = newLibrary();

  while (1) {
    char message_buffer[65536];


    int read_size = recvfrom(s, message_buffer, 65535, 0, (struct sockaddr *) &si_other, &slen);

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
      printf("Got index request!\n");
      size_t bytes_wrote = serializeLibrary(library, message_buffer);

      sendto(s, message_buffer, bytes_wrote, 0, (struct sockaddr*) &si_other, slen);
      continue;
    }
  }

  return 0;
}