cmake_minimum_required(VERSION 4.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

project("scanner")

add_subdirectory(src/scanner)
add_subdirectory(src/mylib)
# add_subdirectory(libs)
# add_subdirectory(tests)