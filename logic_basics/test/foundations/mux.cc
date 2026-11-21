#include "Vmain.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestCase {
  int a, b, s;
  int expected;
};

int main() {
  // Die VCD-Aufzeichnung muss vor dem Erzeugen des Simulationsmodells aktiv sein.
  Verilated::traceEverOn(true);

  Vmain dut;

  // Signale bis zur Hierarchietiefe 99 erfassen.
  // Der Dateipfad ist relativ zu logic_basics/, dem Arbeitsverzeichnis des Tests.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/mux.vcd");

  int sim_time = 0;

  // Wahrheitstabelle: A, B, Auswahl, erwarteter Ausgang.
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

    // Erst die kombinatorische Logik auswerten, dann den Zustand aufzeichnen.
    dut.eval();

    // Der Multiplexer hat keinen Takt: Diese Zeitpunkte bestimmt die Testbench.
    // Sie entsprechen keinen echten Gatterlaufzeiten.
    trace.dump(sim_time);
    sim_time += 10;

    if (dut.y != test.expected) {
      std::cerr << "MUX falsch: A=" << test.a << " B=" << test.b
                << " S=" << test.s << " erwartet=" << test.expected
                << " erhalten=" << int(dut.y) << '\n';
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << "Multiplexer: " << passed << "/" << total << " Tests bestanden\n";
  return 0;
}
