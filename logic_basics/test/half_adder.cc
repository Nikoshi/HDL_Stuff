#include "Vhalf_adder_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestCase {
  int a, b;
  int sum, carry;
};

int main() {
  // Die VCD-Aufzeichnung muss vor dem Erzeugen des Simulationsmodells aktiv sein.
  Verilated::traceEverOn(true);

  Vhalf_adder_top dut;

  // Signale für die Waveform erfassen; Pfad relativ zu logic_basics/.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/half_adder.vcd");

  int sim_time = 0;

  // Wahrheitstabelle: A, B, erwartete Summe, erwarteter Übertrag.
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

    // Ausgänge berechnen und den aktuellen Zustand zur Simulationszeit speichern.
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    if (dut.sum != test.sum) {
      std::cerr << "SUM falsch: A=" << test.a << " B=" << test.b
                << " erwartet=" << test.sum << " erhalten=" << int(dut.sum)
                << '\n';
      return 1;
    }

    if (dut.carry != test.carry) {
      std::cerr << "CARRY falsch: A=" << test.a << " B=" << test.b
                << " erwartet=" << test.carry << " erhalten=" << int(dut.carry)
                << '\n';
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << "Halbaddierer: " << passed << "/" << total << " Tests bestanden\n";
  return 0;
}
