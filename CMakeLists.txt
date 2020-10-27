cmake_minimum_required(VERSION 3.17)
project(ptr-Tidy VERSION 1.0 LANGUAGES CXX)

# Disable in-source builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "FATAL: In-source builds are not allowed.")
endif ()

# So that GitHub build or with multiple LLVM installs, we find the latest installed version of libs
SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

find_package(Clang PATHS "${LLVM_ROOT}/lib/cmake" REQUIRED)
find_package(Boost 1.71 REQUIRED)

add_executable(ptr-tidy src/main.cpp src/Callback.cpp src/IrHandler.cpp src/AstHandler.cpp)
set_target_properties(ptr-tidy PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED True CXX_EXTENSIONS False INTERPROCEDURAL_OPTIMIZATION True)
target_compile_options(ptr-tidy PRIVATE "-fno-rtti" "-fno-exceptions" "-Wall" "-Wextra" "-Wpedantic" "-Wno-unused-parameter")

target_include_directories(ptr-tidy PUBLIC "${CLANG_INCLUDE_DIRS}" "${Boost_INCLUDE_DIRS}")

set(LINK_LIBRARIES clangASTMatchers clangTooling LLVMAnalysis LLVMCore LLVMIRReader)
target_link_libraries(ptr-tidy PUBLIC "${LINK_LIBRARIES}")

add_subdirectory(test)
