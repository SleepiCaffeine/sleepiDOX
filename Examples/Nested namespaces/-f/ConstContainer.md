# ConstContainer

- [T& at(const size_t idx)](#f1)
- [T at(const size_t idx) const](#f2)

- - -
<h2 id="f1"> T& ConstContainer::at(const size_t idx)</h2>

### Description:

Constructor to initialize the container. All values are "zeroed-out", because internally this method invoked `calloc()`
##### Parameters:
- `const size_t` size : the amount of elements this container should store
    


- - -

<h2 id="f2"> T ConstContainer::at(const size_t idx) const</h2>

### Description:

a method that returns an r-value of the element at the specified index. Additionally computed runtime bounds-checking to ensure safe memory access
#### Returns
A copy of the element in `data` at index `idx`
    


- - -

