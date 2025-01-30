## Table of contents : 
- [Wrapper](#1)
- [ConstContainer](#2)
- [Foo](#3)
- [Bar](#4)

- - -
<h3 id=1>Wrapper</h3>

- [char* replaceString(char* src, char* dst, const char* what, const size_t what_len, const char* replacem, const char* replace_len](#f1)
<h3 id=2>ConstContainer</h3>

- [T& at(const size_t idx](#f2)
- [T at(const size_t idx) cons](#f3)
<h3 id=4>Foo</h3>

- [void set_x(const size_t& val](#f4)
- [void set_y(const size_t& val](#f5)
- [size_t sum() noexcep](#f6)
<h3 id=7>Bar</h3>

- [bool waste_time(size_t& s) const noexcep](#f7)

- - -
<h3 id="f1"> char* Wrapper::replaceString(char* src, char* dst, const char* what, const size_t what_len, const char* replacem, const char* replace_len)</h3>

### Description:

Utility function to replace some string with another into buffer `dst`
Not gonna bother explaining.
  Also these comments capture tabs, so be careful making lists, as they can break in markdown.



- - -

<h3 id="f2"> T& ConstContainer::at(const size_t idx)</h3>

### Description:

Constructor to initialize the container. All values are "zeroed-out", because internally this method invoked `calloc()`
##### Parameters:
- `const size_t` size : the amount of elements this container should store
    


- - -

<h3 id="f3"> T ConstContainer::at(const size_t idx) const</h3>

### Description:

a method that returns an r-value of the element at the specified index. Additionally computed runtime bounds-checking to ensure safe memory access
#### Returns
A copy of the element in `data` at index `idx`
    


- - -

<h3 id="f4"> void Foo::set_x(const size_t& val)</h3>

### Description:

Sets the `x` member variable to `val`



- - -

<h3 id="f5"> void Foo::set_y(const size_t& val)</h3>

### Description:

Sets the `y` member variable to `val`



- - -

<h3 id="f6"> size_t Foo::sum() noexcept</h3>

### Description:

Returns the sum of both member variables `x`, and `y`



- - -

<h3 id="f7"> bool Bar::waste_time(size_t& s) const noexcept</h3>

### Description:

Method to do something, but not really anything
#### Parameters
- `size_t& s` : how much time you're willing to waste. Side-effect: because you will never regained wasted time, this function will within itself decrement `s`.
#### Returns
not what you expected
      


- - -



<p style="font-size : 10;">Made using <a href="https://github.com/SleepiCaffeine/sleepiDOX">sleepiDOX</a></p>