#include "Module1.h"

#include <iostream>

namespace ggolbik {
namespace cpp {
namespace shared {
namespace module1 {

using ggolbik::cpp::shared::IModule;

extern "C" {
IModule* create(void) noexcept {
  return static_cast<IModule*>(
      new Module1());
}
void destroy(IModule* p) noexcept {
  if (p != nullptr) {
    delete static_cast<Module1*>(p);
  }
}
}

void Module1::foo() { std::cout << "Module1::foo" << std::endl; }

}  // namespace module1
}  // namespace shared
}  // namespace cpp
}  // namespace ggolbik
