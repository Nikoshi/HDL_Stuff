#include "Vfull_adder_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestCase {
  int a, b, carry_in;
  int sum, carry_out;
};

int main() {
  // Die VCD-Aufzeichnung muss vor dem Erzeugen des Simulationsmodells aktiv sein.
  Verilated::traceEverOn(true);

  Vfull_adder_top dut;

  // Signale für Surfer aufzeichnen; der Pfad ist relativ zu logic_basics/.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/full_adder.vcd");

  int sim_time = 0;

  // Wahrheitstabelle: A, B, Eingangsübertrag, erwartete Summe, Ausgangsübertrag.
  // clang-format off
  const TestCase tests[] = {
      {0, 0, 0, 0, 0},
      {0, 0, 1, 1, 0},
      {0, 1, 0, 1, 0},
      {0, 1, 1, 0, 1},
      {1, 0, 0, 1, 0},
      {1, 0, 1, 0, 1},
      {1, 1, 0, 0, 1},
      {1, 1, 1, 1, 1},
  };
  // clang-format on

  int passed = 0;
  const auto total = sizeof(tests) / sizeof(tests[0]);

  for (const auto &test : tests) {
    // Nur die Eingänge setzen: Die beiden Ausgänge berechnet das Design.
    dut.a = test.a;
    dut.b = test.b;
    dut.carry_in = test.carry_in;

    dut.eval();

    // Ohne Takt legt die Testbench selbst die Simulationszeit fest.
    trace.dump(sim_time);
    sim_time += 10;

    if (dut.sum != test.sum || dut.carry_out != test.carry_out) {
      std::cerr << "Volladdierer falsch: A=" << test.a
                << " B=" << test.b
                << " carry_in=" << test.carry_in
                << " erwartet (sum, carry_out)=(" << test.sum << ", "
                << test.carry_out << ")"
                << " erhalten=(" << int(dut.sum) << ", "
                << int(dut.carry_out) << ")\n";
      trace.close();
      dut.final();
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << "Volladdierer: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
