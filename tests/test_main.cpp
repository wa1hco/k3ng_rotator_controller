#include <exception>
#include <iostream>

#include "test_framework.h"

int main() {
  int passed = 0;
  int failed = 0;

  for (const auto &test_case : testfw::registry()) {
    try {
      test_case.function();
      ++passed;
      std::cout << "[PASS] " << test_case.name << "\n";
    } catch (const std::exception &e) {
      ++failed;
      std::cerr << "[FAIL] " << test_case.name << " - " << e.what() << "\n";
    } catch (...) {
      ++failed;
      std::cerr << "[FAIL] " << test_case.name << " - unknown exception\n";
    }
  }

  std::cout << "\nSummary: " << passed << " passed, " << failed << " failed\n";
  return failed == 0 ? 0 : 1;
}
