#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Book {
  int32_t n, m, k;
  char* name;
} Book;

Book* newBook(int32_t n, int32_t m, int32_t k, char* name);
void freeBook(Book* book);

size_t serializeBook(Book* book, void* dest);
size_t deserializeBook(Book** book, void* source);
void printBook(Book* book, FILE* stream);