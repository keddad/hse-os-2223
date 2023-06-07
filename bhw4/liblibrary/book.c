#include "book.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Book *newBook(int32_t n, int32_t m, int32_t k, char *name) {
  Book *book = malloc(sizeof(Book));

  book->n = n;
  book->m = m;
  book->k = k;

  book->name = malloc(strlen(name) + 1);
  strcpy(book->name, name);

  return book;
}

void freeBook(Book *book) {
  free(book->name);
  free(book);
};

size_t serializeBook(Book *book, void *dest) {
  int32_t *int_ptr = dest;

  *int_ptr = book->n; // is there a better way? like using another language
  int_ptr++;
  *int_ptr = book->m;
  int_ptr++;
  *int_ptr = book->k;
  int_ptr++;

  size_t string_len = strlen(book->name) + 1;
  memcpy(int_ptr, book->name, string_len);

  return sizeof(int32_t) * 3 + string_len;
}

size_t deserializeBook(Book **book, void *source) {
  int32_t *int_ptr = source;

  int32_t n = *int_ptr;
  int32_t m = *(int_ptr + 1);
  int32_t k = *(int_ptr + 2);

  char *name = (char *)(int_ptr + 3);

  *book = newBook(n, m, k, name);

  return sizeof(int32_t) * 3 + strlen(name) + 1;
}

void printBook(Book *book, FILE *stream) {
  fprintf(stream, "Book: %s; n: %d; m: %d; k: %d\n", book->name, book->n,
          book->m, book->k);
}