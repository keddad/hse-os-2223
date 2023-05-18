cmake_minimum_required(VERSION 3.25)

project(brokenlib)

# add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
# add_link_options(-fsanitize=address -fno-omit-frame-pointer)

add_library(liblibrary STATIC 
    liblibrary/book.c
    liblibrary/library.c
)

add_executable(server server.c)
target_link_libraries(server liblibrary)

add_executable(client client.c)
target_link_libraries(client liblibrary)