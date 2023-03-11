/**
 * @file fsm.hpp
 *
 * File that contains a Finite State Machine.
 */
#ifndef PURE_FSM
#define PURE_FSM

#include <cstddef>

/**
 * @bug Clangd can't find `<type_pack.hpp>` header, but can find it by
 * specifying its relative path: `"../../lib/type_pack/include/type_pack.hpp"`.
 * However, clang++ compiles it well.
 */
#include "../../lib/type_pack/include/type_pack.hpp"

#include <type_traits>
#include <utility>
#include <variant>

/**
 * @brief PureFSM library namespace
 */
namespace pure {

  struct none {};

  template <class Source, class Event, class Target, class Action,
            class Guard = none>
  struct transition {
    /** @cond undocumented */
    using source_t = Source;
    using event_t = Event;
    using target_t = Target;
    using action_t = Action;
    using guard_t = Guard;
    /** @endcond */
  };

  /**
   * @brief Typedef to transition
   */
  template <class Source, class Event, class Target, class Action, class Guard>
  using tr = transition<Source, Event, Target, Action, Guard>;

  namespace __details {

    template <class Gd, class Target>
    struct match;

    template <class Guard, class Target>
    inline constexpr bool match_v = match<Guard, Target>::value;

    template <class Guard>
    struct is_logical_guard;

    template <typename T>
    struct unpack {};

    template <typename T, typename... Ts>
    struct unpack<tp::type_pack<T, Ts...>> {
      using type = std::variant<T, Ts...>;
    };

    template <class GuardPack>
    struct unpack_guards;

    template <class Source, class Event, class GuardPack>
    struct tr_cond {};

    template <class TrPack>
    struct unpack_trs {
      using type = tp::empty_pack;
    };

    template <class Tr, class... Trs>
    struct unpack_trs<tp::type_pack<Tr, Trs...>> {
      using source_t = typename Tr::source_t;
      using event_t = typename Tr::event_t;
      using guard_t = typename Tr::guard_t;
      using tr_t = tr_cond<source_t, event_t, guard_t>;

      using type =
          tp::concatenate_t<tp::just_type<tr_t>,
                            typename unpack_trs<tp::type_pack<Trs...>>::type>;
    };
  } // namespace __details

  template <typename... Ts>
  struct transition_table {
    /** @cond undocumented */
    using transitions = tp::type_pack<Ts...>;

    using sources = tp::type_pack<typename Ts::source_t...>;
    using events = tp::type_pack<typename Ts::event_t...>;
    using targets = tp::type_pack<typename Ts::target_t...>;
    using guards_raw = tp::concatenate_t<tp::type_pack<typename Ts::guard_t...>,
                                         tp::just_type<none>>;
    using guards = typename __details::unpack_guards<guards_raw>::type;

    using states = tp::concatenate_t<sources, targets>;

    using state_collection = tp::unique_t<states>;
    using event_collection = tp::unique_t<events>;
    using guard_collection = tp::unique_t<guards>;

    using state_v = typename __details::unpack<state_collection>::type;
    using event_v = typename __details::unpack<event_collection>::type;
    using guard_v = typename __details::unpack<guard_collection>::type;
    /** @endcond */

  private:
    using tr_conds = typename __details::unpack_trs<transitions>::type;
    using test_t = tp::unique_t<tr_conds>;

    static_assert(tp::is_equal<tr_conds, test_t>::value,
                  "Duplicated transitions");
  };

  namespace __details {

    template <class T, class Pack>
    constexpr bool static_check_contains() {
      constexpr bool contains = tp::contains<T, Pack>::value;
      static_assert(contains);
      return contains;
    }

    template <class FSMLogger, class F, typename... Args>
    inline void invoke(FSMLogger& logger, F&& f, Args&&... args) noexcept(
        std::is_nothrow_invocable_v<F, Args...>) {
      if constexpr (std::is_invocable<F, Args...>::value) {
        logger.write("Calling an action...");
        f(std::forward<Args>(args)...);
      }
    }

  } // namespace __details

  class empty_logger {
  public:
    /**
     * @brief Log message, that inspects a type T.
     */
    template <typename T>
    inline void write(const char*) noexcept {}

    /**
     * @brief Log message
     */
    inline void write(const char*) noexcept {}
  };

  template <class Table, class Logger = empty_logger>
  class state_machine {
  private:
    using state_v = typename Table::state_v;
    using event_v = typename Table::event_v;
    using guard_v = typename Table::guard_v;
    using transition_pack = typename Table::transitions;
    using guard_collection = typename Table::guard_collection;

    static constexpr std::size_t m_tr_count = transition_pack::size();

    state_v m_state;
    guard_v m_guard;

    using logger_t = Logger;
    logger_t logger;

    template <class State, class Event, class Guard, class Pack,
              std::size_t Idx>
    struct event_impl {
      template <typename... Args>
      inline void operator()(state_v&, Args&&...) noexcept {}
    };

    template <class State, class Event, class Guard, std::size_t Idx,
              typename T, typename... Ts>
    struct event_impl<State, Event, Guard, tp::type_pack<T, Ts...>, Idx> {
      template <typename... Args>
      inline void operator()(state_v& state, logger_t& log,
                      Args&&... args) {
        using state_t = typename T::source_t;
        using event_t = typename T::event_t;
        using guard_t = typename T::guard_t;
        using target_t = typename T::target_t;
        using action_t = typename T::action_t;

        if constexpr (std::is_same_v<State, state_t> &&
                      std::is_same_v<Event, event_t> &&
                      __details::match_v<Guard, guard_t>) {
          log.template write<target_t>("Change state to ");
          state = target_t {};
          __details::invoke(log, action_t {}, std::forward<Args>(args)...);
        } else
          event_impl<State, Event, Guard, tp::type_pack<Ts...>, Idx + 1> {}(
              state, log, std::forward<Args>(args)...);
      }
    };

  public:
    inline state_machine()
        : m_state(tp::at_t<0, typename Table::sources> {}), m_guard(none {}) {}

    /**
     * @brief Constructor that allows to initialize a logger
     *
     * If the user-defined logger needs some internal data initialization,
     * user can define a template type of State Machine as a reference to a
     * logger and pass a logger by reference; or pass it by value.
     */
    inline state_machine(logger_t custom_logger)
        : m_state(tp::at_t<0, typename Table::sources> {}), m_guard(none {}),
          logger(std::move(custom_logger)) {}

    /**
     * @brief Pass an event to a State Machine
     *
     * @tparam Event event
     * @tparam Args... variadic template type pack of arguments
     *
     * Input impact for State Machine. If the there is a transition, for which
     * current state, current guard and an event are matched, the transition
     * will be performed.
     *
     * If the given event causes a transition, and this transition has an
     * action, it will be called with the arguments `Args...`.
     */
    template <typename Event, typename... Args>
    void event(Args&&... args) {
      logger.template write<Event>("New event: ");
      state_v& ref_state = m_state;
      guard_v& ref_guard = m_guard;
      auto l = [&](const auto& arg) {
        auto gl = [&](const auto& arg2) {
          using event_t = Event;
          using state_t = std::decay_t<decltype(arg)>;
          using guard_t = std::decay_t<decltype(arg2)>;
          event_impl<state_t, event_t, guard_t, transition_pack, 0> {}(
              ref_state, logger, std::forward<Args>(args)...);
        };
        std::visit(gl, ref_guard);
      };
      std::visit(l, m_state);
    }

    /**
     * @brief Calls a state action
     *
     * @tparam Args... variadic template type pack of arguments
     *
     * If the current state type is a functor and it is can be called with the
     * given arguments, it will be called.
     */
    template <typename... Args>
    void action(Args&&... args) {
      logger_t& ref_log = logger;
      auto l = [&](const auto& arg) {
        using state_t = std::decay_t<decltype(arg)>;
        ref_log.template write<state_t>("Attempt to call an action for: ");
        __details::invoke(ref_log, state_t {}, std::forward<Args>(args)...);
      };
      std::visit(l, m_state);
    }

    /**
     * @brief Change current guard
     *
     * @tparam Guard next guard
     */
    template <class Guard>
    inline void guard() {
      if constexpr (__details::static_check_contains<Guard,
                                                     guard_collection>()) {
        logger.template write<Guard>("New guard: ");
        m_guard = Guard {};
      }
    }
  };

  /* guard definitions */

  namespace __details {

    enum class guard_class { noneof, anyof };

    struct logic_guard_base {};

  } // namespace __details

  template <class Guard, class... Guards>
  struct guard_any_of : __details::logic_guard_base {
    /** @cond undocumented */
    static constexpr __details::guard_class type =
        __details::guard_class::anyof;
    using guard_pack = tp::type_pack<Guard, Guards...>;
    using pack = guard_pack;
    /** @endcond */
  };

  template <class Guard, class... Guards>
  struct guard_none_of : __details::logic_guard_base {
    /** @cond undocumented */
    static constexpr __details::guard_class type =
        __details::guard_class::noneof;
    using guard_pack = tp::type_pack<Guard, Guards...>;
    using pack = guard_pack;
    /** @endcond */
  };

  /** @brief Typedef to guard_none_of */
  template <class Guard>
  using not_ = guard_none_of<Guard>;

  /** @brief Typedef to guard_any_of */
  template <class... Guards>
  using any_of = guard_any_of<Guards...>;

  /** @brief Typedef to guard_none_of */
  template <class... Guards>
  using none_of = guard_none_of<Guards...>;

  namespace __details {

    template <class Guard, class Target, typename AlwaysVoid>
    struct match_impl {
      static const bool value = false;
    };

    template <class Guard, class Target>
    struct match_impl<Guard, Target,
                      std::enable_if_t<std::is_same_v<Guard, Target>>> {
      static constexpr bool value = true;
    };

    template <class Guard, class Target>
    struct match_impl<
        Guard, Target,
        std::enable_if_t<
            Target::type == guard_class::anyof &&
            tp::contains<Guard, typename Target::guard_pack>::value>> {
      static constexpr bool value = true;
    };

    template <class Guard, class Target>
    struct match_impl<
        Guard, Target,
        std::enable_if_t<
            Target::type == guard_class::noneof &&
            !tp::contains<Guard, typename Target::guard_pack>::value>> {
      static constexpr bool value = true;
    };

    /*
     * match checks if the guard Guard matches with guard Target.
     * Provides constant member, which is true in the following cases:
     * if Guard == Target
     * if Target == any_of and Guard is appeared in its guard pack
     * if Target == none_of and Guard is not appeared in its guard pack
     *
     * Otherwise the value of member is false.
     */
    template <class Gd, class Target>
    struct match : __details::match_impl<Gd, Target, void> {};

    template <class Guard>
    struct is_logical_guard {
      static constexpr bool value = std::is_base_of_v<logic_guard_base, Guard>;
    };

    template <class Guard, typename AlwaysVoid>
    struct unpack_guard_impl {
      using type = tp::just_type<Guard>;
    };

    template <class Guard>
    struct unpack_guard_impl<Guard,
                             std::enable_if_t<is_logical_guard<Guard>::value>> {
      using type = typename Guard::pack;
    };

    template <class Guard>
    struct unpack_guard : unpack_guard_impl<Guard, void> {};

    template <class GuardPack>
    struct unpack_guards {};

    template <typename T, typename... Ts>
    struct unpack_guards<tp::type_pack<T, Ts...>> {
      using pack = typename unpack_guard<T>::type;
      using next = typename unpack_guards<tp::type_pack<Ts...>>::type;
      using type = tp::concatenate_t<pack, next>;
    };

    template <>
    struct unpack_guards<tp::empty_pack> {
      using type = tp::empty_pack;
    };

  } // namespace __details

} // namespace pure

#endif
