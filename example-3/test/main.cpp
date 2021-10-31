#include <gtest/gtest.h>

#include "../src/library.h"

using namespace ggolbik::cpp::library;

int main(int argc, char **argv)
{
  // The ::testing::InitGoogleTest() function parses the command line for googletest flags, and removes all recognized flags. This allows the user to control a test program’s behavior via various flags,
  ::testing::InitGoogleTest(&argc, argv);
  // RUN_ALL_TESTS() runs all tests
  return RUN_ALL_TESTS();
}

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world") << "the strings don't match";
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

// Tests factorial of 0.
TEST(FactorialTest, HandlesZeroInput) {
  EXPECT_EQ(Factorial(0), 1);
}

// Tests factorial of positive numbers.
TEST(FactorialTest, HandlesPositiveInput) {
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(2), 2);
  EXPECT_EQ(Factorial(3), 6) << "Factorial don't match";
  EXPECT_EQ(Factorial(8), 40320);
}

TEST(ExampleTest, Example1) {
  Example example;
  Example example1 = {};
  Example example2 = {example1};
  Example example3 = {5};
  example1 = example3;

  example1.test();

  example1.~Example();
}

TEST(ExampleTest, Example2) {
  EXPECT_EQ(Factorial(3), 6) << "Factorial don't match";
}
