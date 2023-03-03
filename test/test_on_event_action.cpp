#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <pure/fsm.hpp>
#include <pure/logger.hpp>

enum class output_signal { None, B, C };

struct StateA {};

struct StateB {};

struct StateC {};

struct ActionAB {
  void operator()(output_signal& sig) { sig = output_signal::B; }
};

struct ActionAC {
  void operator()(output_signal& sig) { sig = output_signal::C; }
};

struct EventAB {};

struct EventAC {};

TEST_CASE("On event actions") {
  output_signal signal = output_signal::None;

  using pure::none;
  using pure::tr;

  using table =
      pure::transition_table<tr<StateA, EventAB, StateB, ActionAB, none>,
                             tr<StateA, EventAC, StateC, ActionAC, none>>;
  using logger = pure::stdout_logger<std::cout>;
  pure::state_machine<table, logger> machine;

  SECTION("Attemt to move to State B") {
    machine.event<EventAB>();

    REQUIRE(signal == output_signal::None);
  }

  SECTION("Move to State B") {
    machine.event<EventAB>(signal);

    REQUIRE(signal == output_signal::B);
  }

  SECTION("Move to State C") {
    machine.event<EventAC>(signal);

    REQUIRE(signal == output_signal::C);
  }
}
