#ifndef PURE_FSM
#define PURE_FSM

#include <cstddef>
#include <functional>

// seems it is a bug, and I actually can't fix it by another way
// #include <type_pack.hpp>
#include "../../lib/type_pack/include/type_pack.hpp"

#include <type_traits>
#include <utility>
#include <variant>

namespace pure {

  struct none {};

  namespace __details {

    template <class Gd, class Target>
    struct match;

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

  } // namespace __details

  template <class Source, class Event, class Target, class Action,
            class Guard = none>
  struct transition {
    using source_t = Source;
    using event_t = Event;
    using target_t = Target;
    using action_t = Action;
    using guard_t = Guard;
  };

  template <class Source, class Event, class Target, class Action, class Guard>
  using tr = transition<Source, Event, Target, Action, Guard>;

  template <typename... Ts>
  struct transition_table {
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

  private:
    using test_t = tp::unique_t<transitions>;

    static_assert(std::is_same<test_t, transitions>::value,
                  "Duplicated transitions");
  };

  namespace __details {

    template <class T>
    struct visitor {
      inline auto operator()() {
        return [&](const auto& arg) -> bool {
          using arg_t = std::decay_t<decltype(arg)>;
          return match<arg_t, T>::value;
        };
      }
    };

    template <class T, class Pack>
    constexpr bool static_check_contains() {
      constexpr bool contains = tp::contains<T, Pack>::value;
      static_assert(contains);
      return contains;
    }
  } // namespace __details

  class empty_logger {
  public:
    template <typename T>
    inline void write(const char*) noexcept {}

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

    /*************************************************************************
     * Private methods
     ************************************************************************/

    template <class FSMLogger, class F, typename... Args>
    static void invoke(FSMLogger& logger, F&& f, Args&&... args) noexcept(
        std::is_nothrow_invocable_v<F, Args...>) {
      if constexpr (std::is_invocable<F, Args...>::value) {
        logger.write("Calling an action...");
        std::invoke(f, std::forward<Args>(args)...);
      }
    }

    template <class State, class Event, class Pack, std::size_t Idx>
    struct event_impl {
      template <typename... Args>
      void operator()(state_v&, guard_v&, Args&&...) {}
    };

    template <class State, class Event, std::size_t Idx, typename T,
              typename... Ts>
    struct event_impl<State, Event, tp::type_pack<T, Ts...>, Idx> {
      template <typename... Args>
      void operator()(state_v& state, guard_v& guard, logger_t& log,
                      Args&&... args) {
        using state_t = typename T::source_t;
        using event_t = typename T::event_t;
        using guard_t = typename T::guard_t;
        using target_t = typename T::target_t;
        using action_t = typename T::action_t;

        __details::visitor<guard_t> vis;
        if (std::is_same_v<State, state_t> && std::is_same_v<Event, event_t> &&
            std::visit(vis(), guard)) {
          log.template write<target_t>("Change state to ");
          state = target_t {};
          invoke(log, action_t {}, std::forward<Args>(args)...);
        } else
          event_impl<State, Event, tp::type_pack<Ts...>, Idx + 1> {}(
              state, guard, log, std::forward<Args>(args)...);
      }
    };

  public:
    state_machine()
        : m_state(tp::at_t<0, typename Table::sources> {}), m_guard(none {}) {}

    state_machine(logger_t custom_logger)
        : m_state(tp::at_t<0, typename Table::sources> {}), m_guard(none {}),
          logger(custom_logger) {}

    template <typename Event, typename... Args>
    void event(Args&&... args) {
      logger.template write<Event>("New event: ");
      state_v& ref_state = m_state;
      guard_v& ref_guard = m_guard;
      auto l = [&](const auto& arg) {
        using event_t = Event;
        using state_t = std::decay_t<decltype(arg)>;
        event_impl<state_t, event_t, transition_pack, 0> {}(
            ref_state, ref_guard, logger, std::forward<Args>(args)...);
      };
      std::visit(l, m_state);
    }

    template <typename... Args>
    void action(Args&&... args) {
      logger_t& ref_log = logger;
      auto l = [&](const auto& arg) {
        using state_t = std::decay_t<decltype(arg)>;
        ref_log.template write<state_t>("Attempt to call an action for: ");
        invoke(ref_log, state_t {}, std::forward<Args>(args)...);
      };
      std::visit(l, m_state);
    }

    template <class T>
    inline void guard() {
      if constexpr (__details::static_check_contains<T, guard_collection>()) {
        logger.template write<T>("New guard: ");
        m_guard = T {};
      }
    }
  };

  enum class guard_class { noneof, anyof };

  struct logic_guard_base {};

  template <class Guard, class... Guards>
  struct guard_any_of : logic_guard_base {
    static constexpr guard_class type = guard_class::anyof;
    using guard_pack = tp::type_pack<Guard, Guards...>;
    using pack = guard_pack;
  };

  template <class Guard, class... Guards>
  struct guard_none_of : logic_guard_base {
    static constexpr guard_class type = guard_class::noneof;
    using guard_pack = tp::type_pack<Guard, Guards...>;
    using pack = guard_pack;
  };

  template <class... Guards>
  using any_of = guard_any_of<Guards...>;

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

    template <class Gd, class Target>
    struct match : __details::match_impl<Gd, Target, void> {};

    template <class Guard>
    struct is_logical_guard {
      static constexpr bool value = std::is_base_of_v<logic_guard_base, Guard>;
    };

    template <class GuardPack>
    struct unpack_guards {};

    template <class Guard, typename AlwaysVoid>
    struct unpack_guard {
      using type = tp::just_type<Guard>;
    };

    template <class Guard>
    struct unpack_guard<Guard,
                        std::enable_if_t<is_logical_guard<Guard>::value>> {
      using type = typename Guard::pack;
    };

    template <typename T, typename... Ts>
    struct unpack_guards<tp::type_pack<T, Ts...>> {
      using pack = typename unpack_guard<T, void>::type;
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
