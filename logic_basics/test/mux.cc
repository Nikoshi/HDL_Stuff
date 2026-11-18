#include "Vmain.h"

#include <iostream>

struct TestCase {
  int a, b, s;
  int expected;
};

int main() {
  Vmain dut;

  // A, B, S, Expected
  // clang-format off
  const TestCase tests[] = {
      {0, 0, 0, 0},
      {0, 1, 0, 0},
      {1, 0, 0, 1},
      {1, 1, 0, 1},
      {0, 0, 1, 0},
      {0, 1, 1, 1},
      {1, 0, 1, 0},
      {1, 1, 1, 1},
  };
  // clang-format on

  int passed = 0;
  const auto total = sizeof(tests) / sizeof(tests[0]);

  for (const auto &test : tests) {
    dut.a = test.a;
    dut.b = test.b;
    dut.s = test.s;

    dut.eval();

    if (dut.y != test.expected) {
      std::cerr << "FAILED: A=" << test.a << " B=" << test.b << " S=" << test.s
                << " expected=" << test.expected << " actual=" << int(dut.y)
                << '\n';
      return 1;
    }

    ++passed;
  }

  dut.final();
  std::cout << passed << "/" << total << " tests passed\n";
  return 0;
}
