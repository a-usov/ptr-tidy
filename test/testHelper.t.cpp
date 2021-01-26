#include "Helper.h"
#include "gtest/gtest.h"
#include <fstream>

TEST(TestGetFile, TestCanOpenExistingFile) {
  auto fileName = "/tmp/ptr_tidy_test.txt";

  ASSERT_TRUE(static_cast<bool>(std::ofstream(fileName).put('a'))) << "Error creating test file";

  auto file = getFile(fileName);
  ASSERT_NE(file, std::nullopt);

  ASSERT_FALSE(std::remove(fileName)) << "Error removing test file";
}

TEST(TestGetFile, TestGetNullForMissingFile) {
  auto fileName = "/tmp/ptr_tidy_test.txt";
  auto file = getFile(fileName);
  ASSERT_EQ(file, std::nullopt);
}
