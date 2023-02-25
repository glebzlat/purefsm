#include <cassert>
#include <cstdio>
#include <fsm.hpp>

enum test { a, b };

test t;

struct A {
  void operator()() { t = test::a; }
};

struct B {
  void operator()() { t = test::b; }
};

struct evA {};

struct evB {};

struct None {};

int main() {
  using pure::tr;
  using table = pure::transition_table<tr<A, evA, B, None, None>,
                                       tr<B, evB, A, None, None>>;
  pure::state_machine<table> machine;

  assert(t == test::a);

  machine.event<evA>();
  machine.action();

  assert(t == test::b);

  machine.event<evB>();
  machine.action();

  assert(t == test::a);

  machine.event<evB>();
  machine.action();

  assert(t == test::a);

  return 0;
}
