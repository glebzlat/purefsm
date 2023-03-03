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

struct Event {};

struct GuardA {};

struct GuardB {};

struct GuardC {};

struct GuardD {};

TEST_CASE("Guard any_of") {
  current_state state = current_state::None;

  using pure::any_of;
  using pure::none;
  using pure::tr;

  using table = pure::transition_table<
      tr<StateA, Event, StateB, none, any_of<GuardA, GuardB>>,
      tr<StateA, Event, StateC, none, any_of<GuardC, GuardD>>>;
  using logger = pure::stdout_logger<std::cout>;
  pure::state_machine<table, logger> machine;

  machine.action(state);

  REQUIRE(state == current_state::A);

  SECTION("Attempt to move to State B with guard pure::none") {
    machine.event<Event>();
    machine.action(state);

    REQUIRE_FALSE(state == current_state::B);
  }

  SECTION("Attempt to move to State B with GuardC") {
    machine.guard<GuardC>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE_FALSE(state == current_state::B);
  }

  SECTION("Attempt to move to State B with GuardD") {
    machine.guard<GuardD>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE_FALSE(state == current_state::B);
  }

  SECTION("Moving to State B with GuardA") {
    machine.guard<GuardA>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::B);
  }

  SECTION("Moving to State B with GuardB") {
    machine.guard<GuardB>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::B);
  }

  SECTION("Attempt to move to State C with guard pure::none") {
    machine.event<Event>();
    machine.action(state);

    REQUIRE_FALSE(state == current_state::C);
  }

  SECTION("Attempt to move to State C with GuardA") {
    machine.guard<GuardA>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE_FALSE(state == current_state::C);
  }

  SECTION("Moving to State C with GuardC") {
    machine.guard<GuardC>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }

  SECTION("Moving to State C with GuardD") {
    machine.guard<GuardD>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }
}

TEST_CASE("Guard none_of") {
  current_state state = current_state::None;

  using pure::none;
  using pure::none_of;
  using pure::tr;

  using table = pure::transition_table<
      tr<StateA, Event, StateB, none, none_of<none, GuardA, GuardB>>,
      tr<StateA, Event, StateC, none, none_of<GuardC, GuardD>>>;
  using logger = pure::stdout_logger<std::cout>;
  pure::state_machine<table, logger> machine;

  machine.action(state);

  REQUIRE(state == current_state::A);

  SECTION("Moving to StateC with guard pure::none") {
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }

  SECTION("Moving to StateC with GuardA") {
    machine.guard<GuardA>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }

  SECTION("Moving to StateC with GuardB") {
    machine.guard<GuardB>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::C);
  }

  SECTION("Moving to StateB with GuardC") {
    machine.guard<GuardC>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::B);
  }

  SECTION("Moving to StateB with GuardD") {
    machine.guard<GuardD>();
    machine.event<Event>();
    machine.action(state);

    REQUIRE(state == current_state::B);
  }
}
