cmake_minimum_required(VERSION 3.13)
project(AES_gui)

set(CMAKE_CXX_STANDARD 11)


include_directories(include)
add_library(aes src/aes.c)

link_libraries(aes)
set(LIBRARY_OUTPUT_PATH ../lib)