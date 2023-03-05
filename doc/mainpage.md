# PureFSM

C++ Finite State Machine, written without dynamic memory allocation and RTTI,
only with template metaprogramming. Uses C++17.

Actually this project is a fork of [this project](https://github.com/ALSCode/FSM),
which is inspired by [this lecture](https://www.youtube.com/watch?v=AnOPvP7e7cw).

[Finite-state machine theory](https://en.wikipedia.org/wiki/Finite-state_machine)

The key feature of the project is that transitions (edges between the vertices
of a graph) are stored in a table. This gives much more readability than the
other finite state machine implementations. Table transition representation is
close to a [State/Event table
representation](https://en.wikipedia.org/wiki/Finite-state_machine#State/Event_table).

Transition consists of a source state, which is a current state in a given
discrete moment of time, target state and an event, which will lead to state
change from source to target. Transition has an action, that is performed when
state is changed. Actions allows to easily implement Miley automatons. 

Also transition contains a guard, which is an additional condition to change a
state. It is seems as a logical AND between an event and a guard: 
_if Event AND Guard then perform a transition_. PureFSM allows to specify a sets
of guards using `any_of` and a negated sets using `none_of`. Negated set of
guards means that the transition will be performed, if the current guard is not
appeared in this set.

Suppose that we have the following code snippet:

```cpp
using transition = pure::tr<StateA, Event, StateB, Action, Guard>;
using table = pure::transition_table<transition>;
pure::state_machine<table> machine;
```

This is a state machine with the only one transition: from StateA to StateB with
event Event and guard Guard. This transition will be performed only if the
current state of a machine is StateA, current guard is Guard and machine got an
event Event. When the transition will be performed, Action will be called. You
can also pass arguments to an action. If an action is a functor and it is can be
called with the given arguments, it will be called:

```cpp
machine.event<Event>(args...);
```

Not only transitions can have an action, but the states too. So you can
implement Moore automatons with PureFSM. For to call a state action, you should
use method `pure::state_machine::action`:

```cpp
machine.action(args...);
```

If the current state type is callable and it can be called with arguments 
`args...`, it will be called.

## Cloning and Building

```sh
git clone --recurse-submodules https://github.com/edKotinsky/purefsm.git
cd purefsm
```

### Tests

```sh
cmake -S . -B build/ -D PUREFSM_TESTING=ON
cmake --build build/
```

### Documentation

```sh
cmake -S . -B build/ -D PUREFSM_DOC=ON
cmake --build build/
```

## License

@ref license "MIT"
