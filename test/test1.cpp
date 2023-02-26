#include <cassert>
#include <cstdio>
#include <pure/fsm.hpp>
#include <pure/logger.hpp>
#include <iostream>

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

int main() {
  using pure::tr;
  using pure::none;
  using table = pure::transition_table<tr<A, evA, B, none, none>,
                                       tr<B, evB, A, none, none>>;
  using logger = pure::stdout_logger<std::cout>;
  pure::state_machine<table, logger> machine;

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
