#include "library.h"
#include <iostream>

namespace ggolbik
{
namespace cpp
{
namespace library
{

Example::Example(int field) : field{field} {}

Example &Example::operator=(Example & obj){
  this->field = obj.field;
  return *this;
}

void Example::test(void) const
{
  std::cout << "src" << std::endl;
}

} // namespace library
} // namespace cpp
} // namespace ggolbik