#include "tagged_union.h"

#include <iostream>
#include <string>

using namespace std;

using tagged_union_t = tagged_union::tagged_union_t<int, string>;

class expr_t : public tagged_union_t {
  using tagged_union_t::tagged_union_t;
};

ostream &operator<<(ostream &strm, const expr_t &that) {
  struct visitor_t final : expr_t::visitor_t {
    ostream &strm;
    visitor_t(ostream &strm) : strm(strm) {}
    virtual void operator()() const override { strm << "<void>"; }
    virtual void operator()(const int &that) const override { strm << that; }
    virtual void operator()(const string &that) const override { strm << '"' << that << '"'; }
  };
  that.accept(visitor_t(strm));
  return strm;
}

int main(int, char **) {

  //tagged_union::thing_t<bool, int, int> thing;

  expr_t d;
  cout << "d = " << d << endl;
  expr_t a = string("foo");
  cout << "a = " << a << endl;
  expr_t b = move(a);
  cout
      << "move-construct b from a" << endl
      << "a = " << a << endl
      << "b = " << b << endl;
  expr_t c = b;
  cout
      << "copy-construct c from b" << endl
      << "b = " << b << endl
      << "c = " << c << endl;
  a = move(c);
  cout
      << "move-assign a from c" << endl
      << "a = " << a << endl
      << "c = " << c << endl;
  return 0;
}
