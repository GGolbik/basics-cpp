#pragma once

namespace ggolbik
{
namespace cpp
{
namespace library
{
/**
 * If there is a 
 *      - Copy Constructor
 *      - Copy assignment operator or
 *      - Destructor
 * all three of them must be defined. 
 * If there is a
 *      - Move constructor or
 *      - Move assignment operator
 * all 5 must defined.
 */
class Example
{

public: // Constructor/Destructor/Operator
  /**
   * Default constructor
   */
  Example() = default;
  Example(int field);
  /**
   * Move constructor
   */
  Example(Example &&) = delete;
  /**
   * Move assignment operator
   */
  Example &operator=(Example &&) = delete;
  /**
   * Copy constructor
   */
  Example(const Example &) = default;
  /**
   * Copy assignment operator
   */
  Example &operator=(const Example &);
  /**
   * Destructor
   */
  virtual ~Example() = default;

public: // Public fields and methods
  void test(void) const;

protected: // Protected fields and methods
  int getField();

private: // Private fields and methods
  int field;
};

inline int Example::getField()
{
  return this->field;
}

} // namespace library
} // namespace cpp
} // namespace ggolbik
