#include "Vadd4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

int main() {
  // Das Tracing vor dem Erzeugen der Schaltung aktivieren.
  Verilated::traceEverOn(true);

  Vadd4_top dut;

  // Die aufgezeichneten Signale später in Surfer untersuchen.
  // Das Arbeitsverzeichnis beim Aufruf über Make ist logic_basics/.
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/add4.vcd");

  int sim_time = 0;
  int passed = 0;

  // Exhaustiver Test: Jede der 16 möglichen Zahlen wird mit jeder addiert.
  // Die C++-Addition dient als unabhängige Referenz für die erwartete Summe.
  constexpr int kInputValues = 16;
  constexpr int kTotal = kInputValues * kInputValues;

  for (int a = 0; a < kInputValues; ++a) {
    for (int b = 0; b < kInputValues; ++b) {
      const int expected = a + b;

      dut.a = a;
      dut.b = b;
      dut.eval();

      // Der Addierer ist kombinatorisch: Es gibt hier keine Taktflanken.
      trace.dump(sim_time);
      sim_time += 10;

      if (dut.sum != expected) {
        std::cerr << "4-Bit-Addierer falsch: A=" << a
                  << " B=" << b
                  << " erwartet=" << expected
                  << " erhalten=" << int(dut.sum) << '\n';
        dut.final();
        trace.close();
        return 1;
      }

      ++passed;
    }
  }

  dut.final();
  trace.close();

  std::cout << "4-Bit-Addierer: " << passed << "/" << kTotal
            << " Tests bestanden\n";
  return 0;
}
