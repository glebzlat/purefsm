#ifndef PURE_FSM
#define PURE_FSM

#include <cstddef>
#include <functional>
#include <type_pack.hpp>
#include <type_traits>
#include <utility>
#include <variant>

template <typename T, bool False>
void static_check() {
  static_assert(False);
}

namespace pure {

  template <class Gd1, class Gd2>
  struct match;

  struct none {};

  namespace __details {

    template <typename T>
    struct unpack {};

    template <typename T, typename... Ts>
    struct unpack<tp::type_pack<T, Ts...>> {
      using type = std::variant<T, Ts...>;
    };

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
    using guards = tp::concatenate_t<tp::type_pack<typename Ts::guard_t...>,
                                     tp::just_type<none>>;

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

    template <class F, typename... Args>
    auto invoke_if(F&& f, Args&&... args) noexcept(
        std::is_nothrow_invocable_v<F, Args...>) {
      if constexpr (std::is_invocable<F, Args...>::value)
        return std::invoke(f, std::forward<Args>(args)...);
    }

    template <class T>
    struct visitor {
      inline auto operator()() {
        return [&](const auto& arg) -> bool {
          using arg_t = std::decay_t<decltype(arg)>;
          return std::is_same_v<arg_t, T>;
        };
      }
    };
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

    static constexpr std::size_t m_tr_count = transition_pack::size();

    state_v m_state;
    guard_v m_guard;

    using logger_t = Logger;
    logger_t logger;

    /*************************************************************************
     * Private methods
     ************************************************************************/

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
          __details::invoke_if(action_t {}, std::forward<Args>(args)...);
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
        ref_log.template write<state_t>("State action call: ");
        __details::invoke_if(state_t {}, std::forward<Args>(args)...);
      };
      std::visit(l, m_state);
    }

    template <class T>
    inline void guard() {
      logger.template write<T>("New guard: ");
      m_guard = T {};
    }
  };

} // namespace pure

#endif
