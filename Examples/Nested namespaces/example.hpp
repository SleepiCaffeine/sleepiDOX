// example.hpp
#include <stdio.h>
#include <stdlib.h>

// @sleepiDOX comments outside of a function shouldnt do anything!


namespace Wrapper {


  template <typename T>
  struct ConstContainer {
    T* data = nullptr;
    const size_t n{ 0 };

    /*
@sleepiDOX Constructor to initialize the container. All values are "zeroed-out", because internally this method invoked `calloc()`
##### Parameters:
- `const size_t` size : the amount of elements this container should store
    */
    ConstContainer(const size_t size);

    /*
@sleepiDOX a method that returns an l-value of the element at the specified index. Additionally computed runtime bounds-checking to ensure safe memory access
#### Returns
The reference to the element in `data` at index `idx`
    */
    T& at(const size_t idx);

    /*
@sleepiDOX a method that returns an r-value of the element at the specified index. Additionally computed runtime bounds-checking to ensure safe memory access
#### Returns
A copy of the element in `data` at index `idx`
    */
    T at(const size_t idx) const {
      // oh yeah, you can definitely do this inline, and still have it work!
      if (idx >= size) {
        fprintf(stderr, "Attempted to index beyond the bounds of the ConstContainer. Attempted accessing index %d of a container of size %d\n", idx, size);
        abort();
     }
     return data[idx];
    }
  };

  template <typename T>
  ConstContainer<T>::ConstContainer(const size_t size) {
    data = calloc(size * sizeof(T));
    if (!data) {
      fprintf(stderr, "Failed to allocate the deired number of bytes for the ConstContainer object. Attempted to allocate %d bytes\n", size * sizeof(T));
      abort();
    }
    n = size;
  }

  // @sleepiDOX This comment will not be picked up
  template<typename T>
  inline T& ConstContainer<T>::at(const size_t idx)
  {
    // @sleepiDOX neither will this one, so the token doesn't have to be extremely specific
    if (idx >= size) {
      fprintf(stderr, "Attempted to index beyond the bounds of the ConstContainer. Attempted accessing index %d of a container of size %d\n", idx, size);
      abort();
    }
    return data[idx];
  }





  class Foo {

  private:
    size_t x, y;

    struct Bar {
    private:
      std::nullptr_t Byte;
      unsigned char  Byte2;

/*
@sleepiDOX Method to do something, but not really anything
#### Parameters
- `size_t& s` : how much time you're willing to waste. Side-effect: because you will never regained wasted time, this function will within itself decrement `s`.
#### Returns
not what you expected
      */
      bool waste_time(size_t& s) const noexcept;
    };


  public:
    // Something something, some internal comments
    // Spent hours: 214
    bool valid{ false };


    // @sleepiDOX Sets the `x` member variable to `val`
    void set_x(const size_t& val);

    // @sleepiDOX Sets the `y` member variable to `val`
    void set_y(const size_t& val);

    // @sleepiDOX Returns the sum of both member variables `x`, and `y`
    size_t sum() noexcept;

    // Don't wanna show this one
    char* _SHA265(const char* INPUT, const size_t length);
  };



  /*
@sleepiDOX Utility function to replace some string with another into buffer `dst`
Not gonna bother explaining.
  Also these comments capture tabs, so be careful making lists, as they can break in markdown.
*/
  char* replaceString(char* src, char* dst, const char* what, const size_t what_len, const char* replacem, const char* replace_len);




  void hello();
};