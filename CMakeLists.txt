cmake_minimum_required(VERSION 3.16)
project(ptr-Tidy VERSION 1.0 LANGUAGES CXX)

# Disable in-source builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "FATAL: In-source builds are not allowed.")
endif ()

# So that GitHub build or with multiple LLVM installs, we find the latest installed version of libs
SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

find_package(Clang PATHS "${LLVM_ROOT}/lib/cmake" REQUIRED)
find_package(Boost REQUIRED)

set(LLVM_LIBS Analysis Core IRReader)
set(CLANG_LIBS clangASTMatchers clangTooling)

# On certain distros, clang uses shared lib even if llvm uses static libs.
# So if we detect clang uses shared lib, link llvm against shared lib too
get_target_property(CLANG_LINKS_STATIC clangBasic INTERFACE_LINK_LIBRARIES)
if ("LLVM" IN_LIST CLANG_LINKS_STATIC)
    set(USE_SHARED "USE_SHARED")
else ()
    set(USE_SHARED "")
endif ()

add_executable(ptr-tidy src/main.cpp src/Callback.cpp src/IrHandler.cpp src/AstHandler.cpp)

add_subdirectory(test)

foreach (target IN ITEMS ptr-tidy runTests)
    set_target_properties("${target}" PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED True CXX_EXTENSIONS False INTERPROCEDURAL_OPTIMIZATION True)
    target_compile_definitions("${target}" PUBLIC ${LLVM_DEFINITIONS})
    target_compile_options("${target}" PUBLIC "-fno-rtti" "-fno-exceptions" PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wno-unused-parameter")
    target_include_directories("${target}" PUBLIC "${CLANG_INCLUDE_DIRS}" "${Boost_INCLUDE_DIRS}")
    target_link_libraries("${target}" PUBLIC "${CLANG_LIBS}")
    llvm_config("${target}" "${USE_SHARED}" "${LLVM_LIBS}")
endforeach ()
