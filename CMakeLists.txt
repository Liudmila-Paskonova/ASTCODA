cmake_minimum_required(VERSION 3.22)

project(ErrorDetection)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

# find_package(SQLite3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Eigen3 3.4 REQUIRED NO_MODULE)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(lib)
add_subdirectory(tools)

add_library(tree-lib tree-sitter/lib/src/lib.c tree-sitter-c/src/parser.c tree-sitter-cpp/src/parser.c)
include_directories(
      tree-sitter/lib/src
      tree-sitter/lib/include
)


add_library(tree-sitter STATIC tree-sitter/lib/src/lib.c)
target_include_directories(tree-sitter PUBLIC tree-sitter/lib/include tree-sitter/lib/src)

### SET tree-sitter-cpp
add_library(tree-sitter-cpp STATIC 
  tree-sitter-cpp/src/parser.c 
  tree-sitter-cpp/src/scanner.c 
)
target_include_directories(tree-sitter-cpp PUBLIC tree-sitter-cpp/src)

### SET tree-sitter-c
add_library(tree-sitter-c STATIC 
  tree-sitter-c/src/parser.c 
)
target_include_directories(tree-sitter-c PUBLIC tree-sitter-c/src)

target_link_libraries(tree-lib tree-sitter)
target_link_libraries(tree-lib tree-sitter-c)
target_link_libraries(tree-lib tree-sitter-cpp)