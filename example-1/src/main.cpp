#include <iostream> // Supplies cin, cout, cerr, clog family of functions
#include <memory>   // Smart pointers (std::unique_ptr, std::shared_ptr) enable automatic, exception-safe, object lifetime management.
#include <string>   // strings library includes support for three general types of strings
#include <cstdint>  // Typedef shortcuts like std::uint32_t and std::uint64_t

#include "library.h"

/**
 * @brief The main function is called at program startup after initialization of the non-local objects with static storage duration.
 * See also https://en.cppreference.com/w/cpp/language/main_function
 * @param argc Non-negative value representing the number of arguments passed to the program from the environment in which the program is run.
 * @param argv Pointer to the first element of an array of argc + 1 pointers, of which the last one is null and the previous ones,
 * if any, point to strings that represent the arguments passed to the program from the host environment.
 * If argv[0] is not a null pointer (or, equivalently, if argc > 0), it points to a string that represents the program name,
 * which is empty if the program name is not available from the host environment.
 * @return The values zero and EXIT_SUCCESS indicate successful termination, the value EXIT_FAILURE indicates unsuccessful termination.
 */
int main(int argc, char *argv[])
{
  std::cout << "Hello World!" << std::endl;

  ggolbik::cpp::library::Example example = {1};
  return EXIT_SUCCESS;
}