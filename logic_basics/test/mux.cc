#include "Vmain.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestCase {
  int a, b, s;
  int expected;
};

int main() {
  // Enable waveform tracing before constructing the simulated design.
  Verilated::traceEverOn(true);

  Vmain dut;

  // Register the design's signals (including nested levels up to depth 99).
  // The path is relative to logic_basics/, where the Makefile runs this test.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/mux.vcd");

  int sim_time = 0;

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

    // Recompute the output before recording this input combination.
    dut.eval();

    // The mux has no clock: these are testbench-defined time steps,
    // not physical gate propagation delays.
    trace.dump(sim_time);
    sim_time += 10;

    if (dut.y != test.expected) {
      std::cerr << "FAILED: A=" << test.a << " B=" << test.b << " S=" << test.s
                << " expected=" << test.expected << " actual=" << int(dut.y)
                << '\n';
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << passed << "/" << total << " tests passed\n";
  return 0;
}
