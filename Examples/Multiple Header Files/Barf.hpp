
// Mutual namespace between both files
namespace Mutual {
  
  /*@sleepiDOX You can also re-use the token @sleepiDOX with no issues, could be useful when using a custom token [WIP]*/
  void test();

};




class Barf {

  //@sleepiDOX This works even if the function definiton takes up multiple lines.
  template <class T, class Allocator>
  bool SomeFunction(const T<Allocator>& Item);

  // @sleepiDOX Only the last comment counts, so this line will be completely ignored
  // @sleepiDOX mmmmhmmmmmm...
  void mmmmm();


  // @sleepiDOX ##### we accept you regardless of your function names...
  size_t AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA_____________241544325440124();
};