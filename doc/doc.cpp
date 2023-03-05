namespace pure {
  /**
   * @struct none
   * @brief Struct that defines empty type
   *
   * None can be used as an transition action, if there is no action, as a
   * guard or as an event.
   */

  /**
   * @struct transition
   * @brief Represents a transition between two states
   *
   * A metaprogramming container of types. Transition contains a source
   * state, a target state, an event, which causes state change from source to
   * target and a guard, which determines, is state changing allowed. Also
   * transition contains an action, that is will be performed when this
   * transition is happened.
   */

  /**
   * @struct transition_table
   * @brief Determines the behaviour of an FSM
   *
   * Transition table contains transitions between State Machine states.
   * The first state in the first transition (first column) of a table will be
   * chosen as a beginning state.
   */

  /**
   * @struct empty_logger
   * @brief Empty @ref fsm_logger
   *
   * Dummy logger, that is specified as a State Machine logger by default.
   * If you want to write your own logger, you must to define the same
   * interface.
   *
   * See @ref fsm_logger
   */

  /**
   * @struct state_machine
   * @brief State Machine
   *
   * @tparam Table transition_table
   * @tparam Logger type that provides a logger interface
   */

  /**
   * @struct guard_any_of
   * @brief Guard OR operation
   *
   * @tparam Guard a first guard of a pack
   * @tparam Guards... the rest guards
   *
   * Allows to specify a set of guards for the transition. Must contain at least
   * one guard. Transition will be performed if there is an event match and the
   * current guard is matched with any guard of a set.
   *
   * Be aware that State Machine does not check if guard sets are intersected.
   * E.g if transition table contains two transitions with `any_of` guard sets,
   * and these sets contains at least one guard, that is the same, the behaviour
   * is undefined, if the current guard matched with intersected guard.
   *
   * E.g there are two transitions with guard sets, \f$T_1 = \{a_1, ...,
   * a_n\}\f$ and \f$T_2 = \{b_1, ..., b_n\}\f$. In case if \f$T_1 \cap T_2 = G,
   * G \neq \emptyset\f$, the behaviour is undefined. If the current guard
   * \f$g_c \in G\f$, then it is matched with both transitions.
   */

  /**
   * @struct guard_none_of
   * @brief Guard negation
   *
   * @tparam Guard the first guard of a set
   * @tparam Guards... the rest guards
   *
   * Allows to specify a set of guards, that are not allowed to match the
   * transition. Must contain at least one guard. Guard negation means that the
   * transition is matched, if the _current guard is any, except the negated
   * guards_.
   *
   * Be aware that State Machine does not check, if the union of negation sets
   * is not strictly included in the set of all guards. E.g, there are two
   * transitions with negated guard sets, \f$T_1 = \{a_1, ..., a_n\}\f$ and
   * \f$T_2 = \{b_1, ..., b_n\}\f$, \f$T_1 \cap T_2 = \emptyset\f$. The
   * whole guard set is \f$G\f$, and \f$T_1 \in G, T_2 \in G\f$.
   *
   * In case if \f$T_1 \cup T_2 \neq G\f$, i.e there is at least one guard
   * that is not included in both sets, \f$G - (T_1 \cup T_2) = D, D \neq
   * \emptyset\f$, the behaviour is undefined. If the current guard \f$g_c \in
   * D\f$, then it is matched to both transitions.
   */

  /**
   * @struct stdout_logger
   * @brief State machine stdout logger
   *
   * @tparam stream to write log messages
   *
   * Stream may be `std::cout`, `std::cerr` and `std::clog`.
   *
   * See @ref fsm_logger
   */

  /**
   * @struct user_logger
   * @brief State machine logger with user-defined stream
   *
   * In case if logs needs to be written to a file, and there is no possibility
   * to redirect program standard or error stream, user can apply this logger.
   *
   * See @ref fsm_logger
   */

  /**
   * @page fsm_logger State Machine Logger
   *
   * A State Machine Logger is required to define two methods:
   *
   * Template method `write`, that allows to inspect types
   * ```cpp
   * template <typename T>
   * void write(const char*);
   * ```
   *
   * Type inspectation implemented by compile-time reflection. To get a type
   * name, use `pure::logger::type_name` function. Note that compile-time type
   * reflection is a compiler-dependent feature. It should work with GNU GCC,
   * Clang, MSVC, Intel C++ Compiler and DMC.
   *
   * And non-template method `write`, that allows to just send log messages
   * ```cpp
   * void write(const char*);
   * ```
   *
   * Template `write` method supposed to use compile-time reflection.
   * The possible log message may be `"${str}: ${type}"`.
   *
   * By default State Machine uses `pure::empty_logger`.
   */

  /**
   * @page license License
   *
   * @copyright
   * Copyright (c) 2023 Gleb Zlatanov
   *
   * @par License:
   * MIT License
   *
   * @par
   * Permission is hereby granted, free of charge, to any person obtaining a
   * copy of this software and associated documentation files (the "Software"),
   * to deal in the Software without restriction, including without limitation
   * the rights to use, copy, modify, merge, publish, distribute, sublicense,
   * and/or sell copies of the Software, and to permit persons to whom the
   * Software is furnished to do so, subject to the following conditions:
   *
   * @par
   * The above copyright notice and this permission notice shall be included in
   * all copies or substantial portions of the Software.
   *
   * @par
   * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   * DEALINGS IN THE SOFTWARE.
   */

} // namespace pure
