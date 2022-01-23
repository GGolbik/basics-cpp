#pragma once
#include <ggolbik/cpp/shared/IModule.h>

namespace ggolbik {
namespace cpp {
namespace shared {
namespace module2 {

extern "C" {
extern ggolbik::cpp::shared::IModule* create(void) noexcept;
extern void destroy(ggolbik::cpp::shared::IModule* p) noexcept;
}

class Module2 : public ggolbik::cpp::shared::IModule {
 public:
  Module2(void) = default;
  Module2(const Module2& arg) = default;
  Module2& operator=(const Module2& arg) = default;
  virtual ~Module2(void) = default;

 public:
  void foo(void) override;
};

}  // namespace module2
}  // namespace shared
}  // namespace cpp
}  // namespace ggolbik
