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

## Cloning and Building

```sh
git clone --recurse-submodules https://github.com/edKotinsky/purefsm.git
cd purefsm
cmake -S . -B build/ -D PUREFSM_TESTING=ON
cmake --build build/

# Documentation
cmake -S . -B build/ -D PUREFSM_DOC=ON
cmake --build build/
```

PureFSM uses Catch2 framework for testing. So if you don't want to spend
additional few seconds to download Catch2 submodule, you need to initialize
submodules manually instead of `--recurse-submodules` option for `git clone`:

```sh 
git submodule update --init lib/type_pack
```
