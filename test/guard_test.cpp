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

struct GuardAB {};

struct GuardAC {};

TEST_CASE("Guard test") {
  current_state state = current_state::None;

  using pure::none;
  using pure::tr;

  using table =
      pure::transition_table<tr<StateA, EventAB, StateB, none, GuardAB>,
                             tr<StateA, EventAC, StateC, none, GuardAC>>;
  using logger = pure::stdout_logger<std::cout>;
  pure::state_machine<table, logger> machine;

  machine.action(state);

  REQUIRE(state == current_state::A);

  SECTION("Attempt to move to State B") {
    machine.event<EventAB>();
    machine.action(state);

    REQUIRE(state == current_state::A);

    machine.guard<GuardAC>();
    machine.event<EventAB>();
    machine.action(state);

    REQUIRE(state == current_state::A);
  }

  SECTION("Moving to State B") {
    machine.guard<GuardAB>();
    machine.event<EventAB>();
    machine.action(state);

    REQUIRE(state == current_state::B);
  }

  SECTION("Attempt to move to State C") {
    machine.event<EventAC>();
    machine.action(state);

    REQUIRE(state == current_state::A);

    machine.guard<GuardAB>();
    machine.event<EventAC>();
    machine.action(state);

    REQUIRE(state == current_state::A);
  }

  SECTION("Moving to State C") {
    machine.guard<GuardAC>();
    machine.event<EventAC>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }
}
