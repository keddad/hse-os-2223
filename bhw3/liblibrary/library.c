#include "library.h"
#include "book.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Library *newLibrary() {
  Library *library = malloc(sizeof(Library));

  library->n = 0;
  library->books = NULL;

  return library;
};

void freeLibrary(Library *library) {
  for (int i = 0; i < library->n; ++i) {
    freeBook(library->books[i]);
  }

  free(library->books);
  free(library);
};

size_t serializeLibrary(Library *library, void *dest) {
  int32_t *int_ptr = dest;
  *int_ptr = library->n;
  int_ptr++;

  size_t used_space = sizeof(int32_t);
  char *char_ptr = (char *)int_ptr;

  for (int i = 0; i < library->n; ++i) {
    size_t book_used = serializeBook(library->books[i], char_ptr);

    used_space += book_used;
    char_ptr += book_used;
  }

  return used_space;
};

size_t deserializeLibrary(Library **library, void *source) {
  Library *new_library = newLibrary();
  int32_t *int_ptr = source;

  new_library->n = *int_ptr;
  int_ptr++;
  
  new_library->books = malloc(sizeof(Book *) * new_library->n);

  size_t used_space = sizeof(int32_t);
  char *char_ptr = (char *)int_ptr;

  for (int i = 0; i < new_library->n; ++i) {
    size_t book_used = deserializeBook(&(new_library->books[i]), char_ptr);

    used_space += book_used;
    char_ptr += book_used;
  }

  *library = new_library;

  return used_space;
}

void printLibrary(Library *library, FILE *stream) {
  fprintf(stream, "Currently library has %d books:\n", library->n);

  for (int i = 0; i < library->n; ++i) {
    printBook(library->books[i], stream);
  }
};

int bookComparator(const void *a, const void *b) {
  Book *a_book = *((Book **)a);
  Book *b_book = *((Book **)b);

  return strcmp(a_book->name, b_book->name);
}

void addBook(Library *library, Book *book) {
  library->n += 1;
  library->books = realloc(library->books, sizeof(Book *) * library->n);

  library->books[library->n - 1] = book;

  qsort(library->books, library->n, sizeof(Book *), bookComparator);
};