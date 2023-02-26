#ifndef PUREFSM_LOGGER_HPP
#define PUREFSM_LOGGER_HPP

#include <ostream>

namespace pure {

  namespace __details {

    template <typename T>
    std::string type_name() {
      char const* str = __PRETTY_FUNCTION__;
      while (*str != '=') ++str;
      str += 2;
      std::size_t size = 0;
      while (str[size] != ';' && str[size] != ']') ++size;
      return std::string(str, size);
    }

  } // namespace __details

  template <std::ostream& stream>
  class stdout_logger {
  public:
    stdout_logger() = default;

    template <typename T>
    inline void write(const char* str) {
      stream << str << __details::type_name<T>() << std::endl;
    }

    inline void write(const char* str) { stream << str << std::endl; }
  };

  class user_logger {
  private:
    std::ostream& stream;
  public:
    user_logger() = delete;

    user_logger(std::ostream& stream) : stream(stream) {}

    template <typename T>
    inline void write(const char* str) {
      stream << str << __details::type_name<T>() << std::endl;
    }

    inline void write(const char* str) { stream << str << std::endl; }
  };

} // namespace pure

#endif
