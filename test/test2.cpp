#include <cassert>
#include <fsm.hpp>

enum test { a, b, c };

test t = test::c;

struct A {};

struct B {};

struct ActionA {
  void operator()() { t = test::a; }
};

struct ActionB {
  void operator()() { t = test::b; }
};

struct evA {};

struct evB {};

struct GuardA {};

struct GuardB {};


int main() {
  using pure::tr;
  using pure::none;
  using table = pure::transition_table<tr<A, evA, B, ActionA, GuardA>,
                                       tr<B, evB, A, ActionB, GuardB>>;
  pure::state_machine<table> machine;

  assert(t == test::c);

  machine.event<evA>(); // current guard is `none`, so state does not changed

  assert(t == test::c);

  machine.guard<GuardA>();
  machine.event<evA>(); // guard is `GuardA`, so state changed to B

  assert(t == test::a);

  machine.event<evB>(); // current guard is `GuardA`

  assert(t == test::a);

  machine.guard<GuardB>();
  machine.event<evB>(); // now the current state is A

  assert(t == test::b);

  return 0;
}
