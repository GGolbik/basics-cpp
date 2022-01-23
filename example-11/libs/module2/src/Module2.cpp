#include "Module2.h"

#include <iostream>

namespace ggolbik {
namespace cpp {
namespace shared {
namespace module2 {

using ggolbik::cpp::shared::IModule;

extern "C" {
IModule* create(void) noexcept {
  return static_cast<IModule*>(
      new Module2());
}
void destroy(IModule* p) noexcept {
  if (p != nullptr) {
    delete static_cast<Module2*>(p);
  }
}
}

void Module2::foo() { std::cout << "Module2::foo" << std::endl; }

}  // namespace module2
}  // namespace shared
}  // namespace cpp
}  // namespace ggolbik
