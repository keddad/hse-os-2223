#include <stdint.h>
#include <stdio.h>

#include "liblibrary/book.h"
#include "liblibrary/library.h"

int main() {
  char name1[] = "Cats!";
  char name2[] = "Dogs!";
  char name3[] = "Fishes!";

  Library *lib = newLibrary();

  addBook(lib, newBook(2, 2, 2, name2));
  addBook(lib, newBook(1, 1, 1, name1));
  addBook(lib, newBook(3, 3, 3, name3));

  printLibrary(lib, stdout);

  char buf[4096];

  int32_t serialized = serializeLibrary(lib, buf);

  freeLibrary(lib);

  int32_t deserialized = deserializeLibrary(&lib, buf);

  printLibrary(lib, stdout);

  freeLibrary(lib);

  return 0;
}