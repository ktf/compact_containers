#include <string>
#include <iostream>
#include <iomanip>
#define private public
#include <short_ptr.h>

class A
{
public:
  std::string foo;
};
int
main(char **argv, int argc)
{
  int *fooOrig = new int(0xdeadbeef);
  A *barOrig = new A[100];
  barOrig->foo = "foobar!";
  sptr<int> foo = fooOrig;
  sptr<A> bar = barOrig;
  std::cout << std::hex << fooOrig << " " << foo.id_ << std::endl;
  std::cout << barOrig << " " << bar.id_ << bar->foo << std::endl;
};
