#include <iostream>
#include "../src/library.h"

using namespace ggolbik::cpp::library;

int main(int argc, char *argv[])
{
  std::cout << "Run tests!" << std::endl;

  Example example;
  Example example1 = {};
  Example example2 = {example1};
  Example example3 = {5};
  example1 = example3;

  example1.test();

  example1.~Example();

  return 0;
}