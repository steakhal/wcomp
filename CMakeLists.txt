cmake_minimum_required(VERSION 3.0)
project(wcomp CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
conan_basic_setup(TARGETS)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(GENERATED_SRC "${CMAKE_BINARY_DIR}/generated_src" CACHE INTERNAL "Stores generated files")
file(MAKE_DIRECTORY ${GENERATED_SRC})

add_subdirectory(src)
