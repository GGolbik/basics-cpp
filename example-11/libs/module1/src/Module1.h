#pragma once
#include <ggolbik/cpp/shared/IModule.h>

extern "C" {
extern ggolbik::cpp::shared::IModule* create(void) noexcept;
extern void destroy(ggolbik::cpp::shared::IModule* p) noexcept;
}

namespace ggolbik {
namespace cpp {
namespace shared {
namespace module1 {

class Module1 : public ggolbik::cpp::shared::IModule {
 public:
  Module1(void) = default;
  Module1(const Module1& arg) = default;
  Module1& operator=(const Module1& arg) = default;
  virtual ~Module1(void) = default;

 public:
  void foo(void) override;
};

}  // namespace module1
}  // namespace shared
}  // namespace cpp
}  // namespace ggolbik
