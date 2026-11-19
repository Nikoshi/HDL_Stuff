#include "Vregister1_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestStep {
  int clk, rst, d;
  int expected_q;
  bool check_q;
  const char *description;
};

int main() {
  // Vor der Instanziierung aktivieren, damit auch Registersignale erfasst werden.
  Verilated::traceEverOn(true);

  Vregister1_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/register1.vcd");

  // Erst nach der ersten Reset-Taktflanke ist der Registerzustand definiert.
  // Alle weiteren Schritte prüfen Speicherung, Flanken und synchronen Reset.
  // clang-format off
  const TestStep steps[] = {
      {0, 1, 1, 0, false, "Reset vorbereiten"},
      {1, 1, 1, 0, true, "Synchroner Reset an steigender Flanke"},
      {1, 0, 1, 0, true, "Dateneingang ohne neue Flanke"},
      {0, 0, 1, 0, true, "Fallende Flanke aendert q nicht"},
      {1, 0, 1, 1, true, "D=1 an steigender Flanke uebernehmen"},
      {1, 0, 0, 1, true, "D wechselt bei hohem Takt"},
      {0, 0, 0, 1, true, "Q bleibt bei fallender Flanke"},
      {1, 0, 0, 0, true, "D=0 an steigender Flanke uebernehmen"},
      {1, 1, 1, 0, true, "Reset ohne Flanke aendert q nicht"},
      {0, 1, 1, 0, true, "Reset bei fallender Flanke noch ohne Wirkung"},
      {1, 1, 1, 0, true, "Synchroner Reset setzt q auf 0"},
      {1, 0, 1, 0, true, "Reset deaktivieren ohne Flanke"},
      {0, 0, 1, 0, true, "Q bleibt bei fallender Flanke"},
      {1, 0, 1, 1, true, "D=1 erneut uebernehmen"},
      {1, 1, 0, 1, true, "Resetaktivierung ohne Flanke"},
      {0, 1, 0, 1, true, "Fallende Flanke trotz Reset ohne Effekt"},
      {1, 1, 0, 0, true, "Synchroner Reset loescht gespeicherten Wert"},
  };
  // clang-format on

  int sim_time = 0;
  int passed = 0;
  const auto total = sizeof(steps) / sizeof(steps[0]) - 1;

  for (const auto &step : steps) {
    dut.clk = step.clk;
    dut.rst = step.rst;
    dut.d = step.d;

    // Nach jeder Signaländerung auswerten und die Waveform aufzeichnen.
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    if (!step.check_q) {
      continue;
    }

    if (dut.q != step.expected_q) {
      std::cerr << "1-Bit-Register falsch: " << step.description
                << " (clk=" << step.clk
                << ", rst=" << step.rst
                << ", d=" << step.d
                << "): erwartet q=" << step.expected_q
                << ", erhalten q=" << int(dut.q) << '\n';
      dut.final();
      trace.close();
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << "1-Bit-Register: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
