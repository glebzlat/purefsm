#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <pure/fsm.hpp>
#include <pure/logger.hpp>

enum class current_state { None, A, B, C };

struct StateA {
  void operator()(current_state& state) { state = current_state::A; }
};

struct StateB {
  void operator()(current_state& state) { state = current_state::B; }
};

struct StateC {
  void operator()(current_state& state) { state = current_state::C; }
};

struct EventAB {};

struct EventAC {};

TEST_CASE("Simple test with three states") {
  current_state state = current_state::None;

  using pure::none;
  using pure::tr;

  using table = pure::transition_table<tr<StateA, EventAB, StateB, none, none>,
                                       tr<StateA, EventAC, StateC, none, none>>;
  using logger = pure::stdout_logger<std::cout>;
  pure::state_machine<table, logger> machine;

  machine.action(state);

  REQUIRE(state == current_state::A);

  SECTION("Moving to State B") {
    machine.event<EventAB>();
    machine.action(state);

    REQUIRE(state == current_state::B);
  }

  SECTION("Moving to State C") {
    machine.event<EventAC>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }
}
