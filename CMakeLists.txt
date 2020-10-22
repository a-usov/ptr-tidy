cmake_minimum_required(VERSION 3.17)
project(Ptr-Tidy VERSION 1.0 LANGUAGES CXX)

find_package(Clang PATHS "${LLVM_ROOT}/lib/cmake")

add_executable(ptr-tidy main.cpp)
add_executable(escape escape.cpp)

set_target_properties(ptr-tidy PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED True CXX_EXTENSIONS False INTERPROCEDURAL_OPTIMIZATION True)
set_target_properties(escape PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED True CXX_EXTENSIONS False INTERPROCEDURAL_OPTIMIZATION True)

target_include_directories(ptr-tidy PUBLIC "${CLANG_INCLUDE_DIRS}")
target_include_directories(escape PUBLIC "${CLANG_INCLUDE_DIRS}")

target_link_libraries(ptr-tidy PUBLIC clangASTMatchers clangTooling)
target_link_libraries(escape PUBLIC LLVMCore LLVMIRReader LLVMAnalysis)

target_compile_options(ptr-tidy PRIVATE "-fno-rtti" "-fno-exceptions" "-Wall" "-Wextra" "-Wpedantic" "-Wno-unused-parameter")
target_compile_options(escape PRIVATE "-fno-rtti" "-fno-exceptions" "-Wall" "-Wextra" "-Wpedantic" "-Wno-unused-parameter")