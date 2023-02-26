#include <cassert>
#include <pure/fsm.hpp>
#include <iostream>
#include <pure/logger.hpp>

enum test { a, b, c };

test t = test::a;

struct StateA {};

struct StateB {};

struct StateC {};

struct ActionAB {
  void operator()() { t = test::b; }
};

struct ActionBC {
  void operator()() { t = test::c; }
};

struct Event {};

struct GuardA {};

struct GuardB {};

struct GuardC {};

struct GuardD {};

int main() {
  using logger = pure::stdout_logger<std::cout>;

  using pure::any_of;
  using pure::none;
  using pure::tr;

  using table = pure::transition_table<
      tr<StateA, Event, StateB, ActionAB, any_of<GuardB, GuardD>>,
      tr<StateB, Event, StateC, ActionBC, any_of<GuardA, GuardC>>>;
  pure::state_machine<table, logger> machine;

  assert(t == test::a);

  machine.event<Event>();

  assert(t == test::a);

  machine.guard<GuardA>();
  machine.event<Event>();

  assert(t == test::a);

  machine.guard<GuardC>();
  machine.event<Event>();

  assert(t == test::a);

  machine.guard<GuardB>();
  machine.event<Event>();

  assert(t == test::b);

  machine.event<Event>();

  assert(t == test::b);

  machine.guard<GuardC>();
  machine.event<Event>();

  assert(t == test::c);

  return 0;
}
