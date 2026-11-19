#include "Vregister4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

struct TestStep {
  int clk, rst, enable, d;
  int expected_q;
  bool check_q;
  const char *description;
};

int main() {
  // Für die Auswertung der Registersignale vor der Instanziierung aktivieren.
  Verilated::traceEverOn(true);

  Vregister4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/register4.vcd");

  // Die erste Taktflanke setzt das Register definiert auf 0.
  // Danach werden Enable, beide Taktflanken, alle 16 Datenwerte und
  // der Vorrang des synchronen Resets geprüft.
  // clang-format off
  const TestStep steps[] = {
      {0, 1, 0, 15, 0, false, "Reset vorbereiten"},
      {1, 1, 0, 15, 0, true, "Synchroner Reset trotz deaktiviertem Enable"},
      {1, 0, 1, 10, 0, true, "Neue Daten bei hohem Takt noch nicht uebernehmen"},
      {0, 0, 1, 10, 0, true, "Bei fallender Flanke alten Wert halten"},
      {1, 0, 1, 10, 10, true, "Datenwert 10 an steigender Flanke speichern"},
      {1, 0, 0, 0, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 0, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 0, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 1, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 1, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 1, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 2, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 2, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 2, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 3, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 3, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 3, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 4, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 4, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 4, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 5, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 5, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 5, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 6, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 6, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 6, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 7, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 7, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 7, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 8, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 8, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 8, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 9, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 9, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 9, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 10, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 10, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 10, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 11, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 11, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 11, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 12, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 12, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 12, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 13, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 13, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 13, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 14, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 14, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 14, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 0, 15, 10, true, "Datenwechsel ohne Taktflanke bei Enable=0"},
      {0, 0, 0, 15, 10, true, "Fallende Flanke bei deaktiviertem Enable"},
      {1, 0, 0, 15, 10, true, "Steigende Flanke bei Enable=0 muss alten Wert halten"},
      {1, 0, 1, 0, 10, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 0, 10, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 0, 0, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 1, 0, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 1, 0, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 1, 1, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 2, 1, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 2, 1, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 2, 2, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 3, 2, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 3, 2, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 3, 3, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 4, 3, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 4, 3, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 4, 4, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 5, 4, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 5, 4, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 5, 5, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 6, 5, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 6, 5, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 6, 6, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 7, 6, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 7, 6, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 7, 7, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 8, 7, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 8, 7, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 8, 8, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 9, 8, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 9, 8, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 9, 9, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 10, 9, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 10, 9, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 10, 10, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 11, 10, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 11, 10, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 11, 11, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 12, 11, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 12, 11, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 12, 12, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 13, 12, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 13, 12, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 13, 13, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 14, 13, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 14, 13, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 14, 14, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 0, 1, 15, 14, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 15, 14, true, "Fallende Flanke bei aktiviertem Enable"},
      {1, 0, 1, 15, 15, true, "Neuen Datenwert an steigender Flanke speichern"},
      {1, 1, 0, 3, 15, true, "Reset ohne neue Flanke aktiviert"},
      {0, 1, 0, 3, 15, true, "Reset wirkt nicht auf fallende Flanke"},
      {1, 1, 0, 3, 0, true, "Synchroner Reset trotz Enable=0"},
      {1, 0, 1, 9, 0, true, "Reset deaktivieren ohne Taktflanke"},
      {0, 0, 1, 9, 0, true, "Nach Reset noch kein neuer Wert gespeichert"},
      {1, 0, 1, 9, 9, true, "Wert nach Reset wieder speichern"},
      {1, 1, 1, 5, 9, true, "Reset aktivieren bei hohem Takt"},
      {0, 1, 1, 5, 9, true, "Reset wirkt weiterhin nicht auf fallende Flanke"},
      {1, 1, 1, 5, 0, true, "Reset hat Vorrang vor aktiviertem Enable"},
  };
  // clang-format on

  int sim_time = 0;
  int passed = 0;
  constexpr int kUncheckedSetupSteps = 1;
  const auto total = sizeof(steps) / sizeof(steps[0]) - kUncheckedSetupSteps;

  for (const auto &step : steps) {
    dut.clk = step.clk;
    dut.rst = step.rst;
    dut.enable = step.enable;
    dut.d = step.d;

    // Jedes Signalpaket simulieren und für Surfer aufzeichnen.
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    if (!step.check_q) {
      continue;
    }

    if (dut.q != step.expected_q) {
      std::cerr << "4-Bit-Register falsch: " << step.description
                << " (clk=" << step.clk
                << ", rst=" << step.rst
                << ", enable=" << step.enable
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

  std::cout << "4-Bit-Register: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
