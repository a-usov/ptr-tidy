find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(runTests testPtrRewriter.t.cpp ../src/PtrRewriter.cpp)
target_include_directories(runTests PUBLIC "../src")
target_link_libraries(runTests PUBLIC "${GTEST_BOTH_LIBRARIES}" Threads::Threads)
