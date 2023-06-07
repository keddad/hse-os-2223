#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "book.h"

typedef struct Library {
    int32_t n;
    Book** books;
} Library;

Library* newLibrary();
void freeLibrary(Library* library);

size_t serializeLibrary(Library* library, void* dest);
size_t deserializeLibrary(Library** library, void* source);
void printLibrary(Library* library, FILE* stream);

void addBook(Library* library, Book* book);