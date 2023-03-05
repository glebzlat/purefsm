/**
 * @file logger.hpp
 *
 * File logger.hpp provides logger types and utilities for PureFSM.
 */
#ifndef PUREFSM_LOGGER_HPP
#define PUREFSM_LOGGER_HPP

#include <ostream>

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
 * And non-template method `write`, that allows to just send log messages
 * ```cpp
 * void write(const char*);
 * ```
 *
 * Template `write` method supposed to use compile-time reflection.
 * The possible log message may be `"${str}: ${type}"`.
 */

namespace pure {

  /**
   * @brief Logger utilities namespace
   *
   * See @ref fsm_logger
   */
  namespace logger {

    /**
     * @fn type_name
     *
     * @tparam T type
     *
     * @brief Compile-time reflection
     *
     * Returns a string that is a name of a type T. Compiler dependent.
     */

#if defined(__GNUC__) || defined(__MINGW32__) || defined(__clang__) ||         \
    defined(__INTEL_COMPILER) || (defined(__ICC) && (__ICC >= 600)) ||         \
    (defined(__DMC__) && __DMC__ >= 0x810)

    template <typename T>
    std::string type_name() {
      using size_type = typename std::string::size_type;
      char const* str = __PRETTY_FUNCTION__;
      while (*str != '=') ++str;
      str += 2;
      size_type size = 0;
      while (str[size] != ';' && str[size] != ']') ++size;
      return std::string(str, size);
    }

#elif defined(__MSC_VER) || defined(__FUNCSIG__)

    template <typename T>
    std::string type_name() {
      using size_type = typename std::string::size_type;
      const std::string s = __FUNCSIG__;
      const std::string fname = "type_name<";
      const std::string rest = ">(void)";
      size_type pos = s.find(fname) + fname.size();
      size_type count = s.find(rest) - pos;
      return std::string(s, pos, count);
    }

#else

  #warning                                                                     \
      "Your compiler does not support the reflection required by the logger"

    template <typename T>
    std::string type_name() {
      return {};
    }

#endif

  } // namespace logger

  /**
   * @brief State machine stdout logger
   *
   * @tparam stream to write log messages
   *
   * Stream may be `std::cout`, `std::cerr` and `std::clog`.
   *
   * See @ref fsm_logger
   */
  template <std::ostream& stream>
  class stdout_logger {
  public:
    stdout_logger() = default;

    template <typename T>
    inline void write(const char* str) {
      stream << str << logger::type_name<T>() << std::endl;
    }

    inline void write(const char* str) { stream << str << std::endl; }
  };

  /**
   * @brief State machine logger with user-defined stream
   *
   * In case if logs needs to be written to a file, and there is no possibility
   * to redirect program standard or error stream, user can apply this logger.
   *
   * See @ref fsm_logger
   */
  class user_logger {
  private:
    std::ostream& stream;
  public:
    user_logger() = delete;

    /**
     * @brief Constructs user_logger with a stream given
     */
    user_logger(std::ostream& stream) : stream(stream) {}

    /**
     * @brief Constructs user_logger from a reference to another user_logger
     */
    user_logger(user_logger const& other) : stream(other.stream) {}

    /**
     * @brief Constructs user_logger from another user_logger
     */
    user_logger(user_logger&& other) : stream(other.stream) {}

    template <typename T>
    inline void write(const char* str) {
      stream << str << logger::type_name<T>() << std::endl;
    }

    inline void write(const char* str) { stream << str << std::endl; }
  };

} // namespace pure

#endif
