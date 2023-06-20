# PureFSM

[![clang](https://github.com/glebzlat/purefsm/actions/workflows/clang.yml/badge.svg)](https://github.com/edKotinsky/purefsm/actions/workflows/clang.yml)
 [![gcc](https://github.com/glebzlat/purefsm/actions/workflows/gcc.yml/badge.svg)](https://github.com/edKotinsky/purefsm/actions/workflows/gcc.yml)
 [![Docs](https://github.com/glebzlat/purefsm/actions/workflows/docs.yml/badge.svg)](https://github.com/edKotinsky/purefsm/actions/workflows/docs.yml)

C++ finite state machine, written without dynamic memory allocation and RTTI -
only with _pure_ template metaprogramming. Written in C++17.

## Cloning and Building

```sh
git clone --recurse-submodules https://github.com/glebzlat/purefsm.git
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

[MIT](./LICENSE)
