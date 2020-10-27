find_package(GTest REQUIRED)

add_executable(runTests testIrHandler.t.cpp ../src/IrHandler.cpp)

target_link_libraries(runTests "${GTEST_BOTH_LIBRARIES}" "${LINK_LIBRARIES}")

target_include_directories(runTests PUBLIC "../src" "${CLANG_INCLUDE_DIRS}" "${Boost_INCLUDE_DIRS} ")
