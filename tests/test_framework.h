#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace testfw {

using TestFunction = void (*)();

struct TestCase {
  std::string name;
  TestFunction function;
};

inline std::vector<TestCase> &registry() {
  static std::vector<TestCase> tests;
  return tests;
}

class Registrar {
 public:
  Registrar(const char *name, TestFunction function) {
    registry().push_back({name, function});
  }
};

inline void fail(const char *file, int line, const std::string &message) {
  std::ostringstream oss;
  oss << file << ":" << line << " " << message;
  throw std::runtime_error(oss.str());
}

}  // namespace testfw

#define TEST_CASE(name)                                \
  static void name();                                  \
  static testfw::Registrar name##_registrar(#name, name); \
  static void name()

#define REQUIRE_TRUE(condition)                                               \
  do {                                                                        \
    if (!(condition)) {                                                       \
      testfw::fail(__FILE__, __LINE__, "Expected true: " #condition);        \
    }                                                                         \
  } while (0)

#define REQUIRE_FALSE(condition)                                              \
  do {                                                                        \
    if ((condition)) {                                                        \
      testfw::fail(__FILE__, __LINE__, "Expected false: " #condition);       \
    }                                                                         \
  } while (0)

#define REQUIRE_EQ(expected, actual)                                          \
  do {                                                                        \
    const auto expected_value = (expected);                                   \
    const auto actual_value = (actual);                                       \
    if (!(expected_value == actual_value)) {                                  \
      std::ostringstream oss;                                                 \
      oss << "Expected " << #actual << " == " << #expected;                 \
      testfw::fail(__FILE__, __LINE__, oss.str());                            \
    }                                                                         \
  } while (0)

#endif  // TEST_FRAMEWORK_H
