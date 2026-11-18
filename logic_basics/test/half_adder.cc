#include "Vhalf_adder_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestCase {
  int a, b;
  int sum, carry;
};

int main() {
  // Die Aufzeichnung aktivieren, bevor das simulierte Design erzeugt wird.
  Verilated::traceEverOn(true);

  Vhalf_adder_top dut;

  // Signale des Designs bis zu einer Hierarchietiefe von 99 registrieren.
  // Der Dateipfad ist relativ zu logic_basics/, wo das Makefile den Test
  // ausführt.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/half_adder.vcd");

  int sim_time = 0;

  // A, B, Auswahl, erwarteter Ausgang
  // clang-format off
  const TestCase tests[] = {
      {0, 0, 0, 0},
      {0, 1, 1, 0},
      {1, 0, 1, 0},
      {1, 1, 0, 1},

  };
  // clang-format on

  int passed = 0;
  const auto total = sizeof(tests) / sizeof(tests[0]);

  for (const auto &test : tests) {
    dut.a = test.a;
    dut.b = test.b;

    dut.eval();

    // Erst den Ausgang neu berechnen, dann die aktuelle Eingangskombination
    // aufzeichnen.
    dut.eval();

    // Der Multiplexer hat keinen Takt: Die Testbench legt die Zeitschritte
    // fest. Sie entsprechen keinen tatsächlichen Gatterlaufzeiten.
    trace.dump(sim_time);
    sim_time += 10;

    if (dut.sum != test.sum) {
      std::cerr << "SUM falsch: erwartet " << test.sum << ", erhalten "
                << int(dut.sum) << '\n';
      return 1;
    }

    if (dut.carry != test.carry) {
      std::cerr << "CARRY falsch: erwartet " << test.carry << ", erhalten "
                << int(dut.carry) << '\n';
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << passed << "/" << total << " tests passed\n";
  return 0;
}
