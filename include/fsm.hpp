#ifndef PURE_FSM
#define PURE_FSM

#include <cstddef>
#include <functional>
#include <type_pack.hpp>
#include <type_traits>
#include <variant>

template <typename T, bool False>
void static_check() {
  static_assert(False);
}

namespace pure {

  template <class Gd1, class Gd2>
  struct match;

  namespace __details {

    template <typename T>
    struct unpack {};

    template <typename T, typename... Ts>
    struct unpack<tp::type_pack<T, Ts...>> {
      using type = std::variant<T, Ts...>;
    };

  } // namespace __details

  struct transition_base {};

  template <class Source, class Event, class Target, class Action, class Guard>
  struct transition : transition_base {
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
    using guards = tp::type_pack<typename Ts::guard_t...>;

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

  template <class Source, class Event>
  struct transition_key {
    using source_t = Source;
    using event_t = Event;
  };

  namespace __details {

    template <class T>
    struct extract_transition_keys {};

    template <>
    struct extract_transition_keys<tp::empty_pack> {
      using type = tp::empty_pack;
    };

    template <typename T, typename... Ts>
    struct extract_transition_keys<tp::type_pack<T, Ts...>> {
      using source_t = typename T::source_t;
      using event_t = typename T::event_t;
      using key = transition_key<source_t, event_t>;
      using jt_key = tp::just_type<key>;

      using type = tp::concatenate_t<
          jt_key, typename extract_transition_keys<tp::type_pack<Ts...>>::type>;
    };

    template <class Target, class Action>
    struct transition_data {
      using target_t = Target;
      using action_t = Action;
    };

    struct none_transition {};

    template <class Table>
    class transition_map {
    private:
      using transitions = typename Table::transitions;
      using keys =
          typename __details::extract_transition_keys<transitions>::type;
      using guards = typename Table::guards;
      using targets = typename Table::targets;

      static constexpr std::size_t npos = transitions::size();

      // template <class Key, std::size_t Idx>
      // struct get_impl {
      //   static constexpr std::size_t idx = tp::find<Key, keys, Idx>::value;
      //   using target = tp::at_t<idx, targets>;
      //   using transition = tp::at_t<idx, transitions>;
      //   using action = typename transition::action_t;
      //   using type = transition_data<target, action>;
      // };

    public:
      template <class St, class Ev>
      auto get() {
        using key = transition_key<St, Ev>;
        static constexpr std::size_t idx = tp::find<key, keys>::value;
        if constexpr (idx == npos) return none_transition {};
        else {
          using target = tp::at_t<idx, targets>;
          using transition = tp::at_t<idx, transitions>;
          using action = typename transition::action_t;
          using type = transition_data<target, action>;
          return type {};
        }
      }
    };

  } // namespace __details

  template <class Table>
  class state_machine {
  private:
    using state_v = typename Table::state_v;
    using event_v = typename Table::event_v;
    using transition_pack = typename Table::transitions;

    using map_t = __details::transition_map<Table>;
    map_t m_map;
    state_v m_state;

    /*************************************************************************
     * Private methods
     ************************************************************************/

    template <typename T, typename... Args>
    inline auto event_impl(Args&&... args) -> std::enable_if_t<
        !std::is_same<T, __details::none_transition>::value> {
      using action_t = typename T::action_t;
      using target_t = typename T::target_t;
      m_state = target_t {};
      if constexpr (std::is_invocable<action_t, Args...>::value)
        std::invoke(action_t {}, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    inline auto event_impl(Args&&...) noexcept -> std::enable_if_t<
        std::is_same<T, __details::none_transition>::value> {}

  public:
    state_machine() : m_state(tp::at_t<0, typename Table::sources> {}) {}

    template <typename Event, typename... Args>
    void event(Args&&... args) {
      map_t& ref_map = m_map;
      auto l = [&](const auto& arg) {
        using event_t = Event;
        using state_t = std::decay_t<decltype(arg)>;
        using data = decltype(ref_map.template get<state_t, event_t>());
        event_impl<data>(std::forward<Args>(args)...);
      };
      std::visit(l, m_state);
    }

    template <typename... Args>
    void action(Args&&... args) {
      auto l = [&](const auto& arg) {
        using state_t = std::decay_t<decltype(arg)>;
        if constexpr (std::is_invocable<state_t, Args...>::value)
          std::invoke(state_t {}, std::forward<Args>(args)...);
      };
      std::visit(l, m_state);
    }
  };

} // namespace pure

#endif
