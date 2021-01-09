find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(runTests testIrHandler.t.cpp ../src/IrHandler.cpp testHelper.t.cpp)
target_include_directories(runTests PUBLIC "../src")
target_link_libraries(runTests PUBLIC "${GTEST_BOTH_LIBRARIES}" Threads::Threads)
