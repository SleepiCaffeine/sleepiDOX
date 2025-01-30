// example.hpp

class Foo {
  size_t x, y;
public:
  // @sleepiDOX Sets the `x` member variable to `val`
  void set_x(const size_t& val);

  // @sleepiDOX Sets the `y` member variable to `val`
  void set_y(const size_t& val);

  // @sleepiDOX Returns the sum of both member variables `x`, and `y`
  size_t sum() noexcept;

  // Don't wanna show this one
  char* _SHA265(const char* INPUT, const size_t length);
};