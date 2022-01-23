#include <ggolbik/cpp/shared/IModule.h>

#include <cstdlib>  // EXIT_SUCCESS; EXIT_FAILURE
#include <memory>   // std::shared_ptr; std::make_shared
#include <string>  // strings library includes support for three general types of strings
#include <vector>  // std::vector

#include "ggolbik/cpp/shared/SharedLibrary.h"

using namespace ggolbik::cpp::shared;

using ModuleSharedLibrary = SharedLibrary<IModule>;
using ModuleSharedLibraryPtr = std::shared_ptr<ModuleSharedLibrary>;
using ModuleSharedLibraryList = std::vector<ModuleSharedLibraryPtr>;

ModuleSharedLibraryList load(const std::vector<std::string>& libraryNames);
std::vector<ModuleSharedLibrary::InstancePtr> create(
    ModuleSharedLibraryList& sharedLibraries);
bool execute(std::vector<ModuleSharedLibrary::InstancePtr> instances);

/**
 * @brief The main function is called at program startup after initialization of
 * the non-local objects with static storage duration. See also
 * https://en.cppreference.com/w/cpp/language/main_function
 * @param argc Non-negative value representing the number of arguments passed to
 * the program from the environment in which the program is run.
 * @param argv Pointer to the first element of an array of argc + 1 pointers, of
 * which the last one is null and the previous ones, if any, point to strings
 * that represent the arguments passed to the program from the host environment.
 * If argv[0] is not a null pointer (or, equivalently, if argc > 0), it points
 * to a string that represents the program name, which is empty if the program
 * name is not available from the host environment.
 * @return The values zero and EXIT_SUCCESS indicate successful termination, the
 * value EXIT_FAILURE indicates unsuccessful termination.
 */
int main(int argc, char* argv[]) {
  // parse arguments
  std::vector<std::string> libraryNames;
  for (int i = 1; i < argc; i++) {
    libraryNames.push_back(argv[i]);
  }
  if (libraryNames.empty()) {
    std::cerr << "You must pass library names as arguments." << std::endl
              << "The libraries must provide the following functions:"
              << std::endl
              << "- 'IModule* create(void) noexcept;'" << std::endl
              << "- 'void destroy(IModule* p) noexcept;'" << std::endl;
    return EXIT_FAILURE;
  }

  // load libraries
  auto sharedLibraries = load(libraryNames);
  if (sharedLibraries.empty()) {
    std::cerr << "Failed to load any library" << std::endl;
    return EXIT_FAILURE;
  }

  // create instances
  auto instances = create(sharedLibraries);
  if (instances.empty()) {
    std::cerr << "Failed to create any instance" << std::endl;
    return EXIT_FAILURE;
  }

  // execute instance methods
  if (execute(instances)) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

ModuleSharedLibraryList load(const std::vector<std::string>& libraryNames) {
  ModuleSharedLibraryList sharedLibraries;
  for (const std::string& libraryName : libraryNames) {
    ModuleSharedLibraryPtr libPtr =
        std::make_shared<ModuleSharedLibrary>(libraryName);
    std::string errorMessage;
    if (!libPtr->load(errorMessage)) {
      std::cerr << errorMessage << std::endl;
    } else {
      sharedLibraries.push_back(libPtr);
    }
  }
  return sharedLibraries;
}

std::vector<ModuleSharedLibrary::InstancePtr> create(
    ModuleSharedLibraryList& sharedLibraries) {
  std::vector<ModuleSharedLibrary::InstancePtr> instances;
  for (auto& sharedLibrary : sharedLibraries) {
    std::string errorMessage;
    std::shared_ptr<IModule> instance = sharedLibrary->create(errorMessage);
    if (!instance) {
      std::cerr << errorMessage << std::endl;
    } else {
      instances.push_back(instance);
    }
  }
  return instances;
}

bool execute(std::vector<ModuleSharedLibrary::InstancePtr> instances) {
  for (auto& instance : instances) {
    instance->foo();
  }
  return true;
}