cmake_minimum_required(VERSION 3.17)
project(Project VERSION 1.0 LANGUAGES CXX)

find_package(Clang PATHS "${LLVM_ROOT}/lib/cmake")

add_executable(project main.cpp)

set_target_properties(project PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED True CXX_EXTENSIONS False)

target_include_directories(project PUBLIC "${CLANG_INCLUDE_DIRS}")

target_link_libraries(project PUBLIC clangASTMatchers)
