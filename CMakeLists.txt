cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pureFSM LANGUAGES CXX)

option(PUREFSM_TESTING "Build and run tests" OFF)
option(PUREFSM_DOC "Build documentation" OFF)

set(LIB_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(PureFSM INTERFACE)
target_include_directories(PureFSM INTERFACE include)
target_compile_options(PureFSM INTERFACE
    -Wall
    -Wpedantic
    -Wextra
)

add_subdirectory(${LIB_DIRECTORY}/type_pack)
target_link_libraries(PureFSM INTERFACE type_pack)

option(FSM_TESTING "Build and run tests" OFF)
option(FSM_DOC "Build documentation" OFF)

if (PUREFSM_TESTING)
    add_subdirectory(${LIB_DIRECTORY}/Catch2)
    add_subdirectory(test)
endif()

if (PUREFSM_DOC)
    add_subdirectory(examples)
endif()
