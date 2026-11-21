#include "Vadd2_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestCase {
  int a, b;
  int expected;
};

int main() {
  // Bereits vor dem Erzeugen des Simulationsmodells Tracing aktivieren.
  Verilated::traceEverOn(true);

  Vadd2_top dut;

  // Die Signalverläufe lassen sich später in der Surfer-Webapp öffnen.
  // Das Makefile startet den Test aus dem Verzeichnis logic_basics/.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/add2.vcd");

  int sim_time = 0;

  // Alle 16 Kombinationen zweier 2-Bit-Zahlen.
  // Spalte 3 enthält jeweils die vollständige 3-Bit-Summe (0 bis 6).
  // clang-format off
  const TestCase tests[] = {
      {0, 0, 0},
      {0, 1, 1},
      {0, 2, 2},
      {0, 3, 3},
      {1, 0, 1},
      {1, 1, 2},
      {1, 2, 3},
      {1, 3, 4},
      {2, 0, 2},
      {2, 1, 3},
      {2, 2, 4},
      {2, 3, 5},
      {3, 0, 3},
      {3, 1, 4},
      {3, 2, 5},
      {3, 3, 6},
  };
  // clang-format on

  int passed = 0;
  const auto total = sizeof(tests) / sizeof(tests[0]);

  for (const auto &test : tests) {
    // Die Testbench setzt nur Eingänge und liest danach das berechnete Ergebnis.
    dut.a = test.a;
    dut.b = test.b;

    dut.eval();

    // Diese Schaltung hat keinen Takt: Die Zeitachse stammt aus der Testbench.
    trace.dump(sim_time);
    sim_time += 10;

    if (dut.sum != test.expected) {
      std::cerr << "2-Bit-Addierer falsch: A=" << test.a
                << " B=" << test.b
                << " erwartet=" << test.expected
                << " erhalten=" << int(dut.sum) << '\n';
      trace.close();
      dut.final();
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << "2-Bit-Addierer: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
