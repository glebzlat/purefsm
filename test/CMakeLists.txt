project(purefsmtest LANGUAGES CXX)

include(CTest)

# list of all test targets as a dependency for a test launcher target
set(TEST_LIST)
set(TEST_DEPENDENCY PureFSM)

macro(add_test_exec TEST_NAME)
    add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE
        ${TEST_DEPENDENCY}
        Catch2::Catch2WithMain)
    list(APPEND TEST_LIST ${TEST_NAME})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

add_test_exec(SmokeTest smoke_test.cpp)
add_test_exec(GuardTest guard_test.cpp)
add_test_exec(LogicGuards test_logic_guards.cpp)
add_test_exec(OnEventAction test_on_event_action.cpp)

add_custom_target(MakeTest ALL
    ctest --output-on-failure --test-timeout 10
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
    VERBATIM
    DEPENDS ${TEST_LIST}
)

enable_testing()
