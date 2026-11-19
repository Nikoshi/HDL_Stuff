#include "Vregister4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>
#include <vector>

struct TestStep {
  int clk, rst, enable, d;
  int expected_q;
  bool check_q;
  const char *description;
};

int main() {
  // Das Tracing vor dem Erzeugen der Schaltung aktivieren.
  Verilated::traceEverOn(true);

  Vregister4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/register4.vcd");

  // Schritt 1: Synchronen Reset auslösen und anschließend einen Wert speichern.
  // Der erste Schritt initialisiert nur die Eingänge; q ist noch undefiniert.
  std::vector<TestStep> steps = {
      {0, 1, 0, 15, 0, false, "Reset vorbereiten"},
      {1, 1, 0, 15, 0, true, "Reset trotz deaktiviertem Enable"},
      {1, 0, 1, 10, 0, true, "Datenwechsel bei hohem Takt"},
      {0, 0, 1, 10, 0, true, "Fallende Flanke speichert nichts"},
      {1, 0, 1, 10, 10, true, "Wert 10 speichern"},
  };

  // Schritt 2: Bei deaktiviertem Enable alle 16 Datenwerte anlegen.
  // Selbst an steigenden Flanken muss der gespeicherte Wert 10 bleiben.
  for (int d = 0; d < 16; ++d) {
    steps.push_back({1, 0, 0, d, 10, true, "Datenwechsel ohne Flanke"});
    steps.push_back({0, 0, 0, d, 10, true, "Fallende Flanke ohne Enable"});
    steps.push_back({1, 0, 0, d, 10, true, "Steigende Flanke ohne Enable"});
  }

  // Schritt 3: Mit aktiviertem Enable jeden der 16 Werte speichern.
  // Zwischen den steigenden Flanken bleibt der jeweils vorherige Wert gültig.
  int previous = 10;
  for (int d = 0; d < 16; ++d) {
    steps.push_back({1, 0, 1, d, previous, true, "Datenwechsel bei hohem Takt"});
    steps.push_back({0, 0, 1, d, previous, true, "Fallende Flanke mit Enable"});
    steps.push_back({1, 0, 1, d, d, true, "Steigende Flanke mit Enable"});
    previous = d;
  }

  // Schritt 4: Der synchrone Reset ist sowohl bei Enable=0 als auch
  // bei Enable=1 wirksam, aber nie ohne steigende Taktflanke.
  steps.insert(steps.end(), {
      {1, 1, 0, 3, 15, true, "Reset ohne Taktflanke aktivieren"},
      {0, 1, 0, 3, 15, true, "Fallende Flanke mit Reset"},
      {1, 1, 0, 3, 0, true, "Reset hat Vorrang vor Enable=0"},
      {1, 0, 1, 9, 0, true, "Reset deaktivieren ohne Flanke"},
      {0, 0, 1, 9, 0, true, "Wert nach Reset halten"},
      {1, 0, 1, 9, 9, true, "Nach Reset erneut speichern"},
      {1, 1, 1, 5, 9, true, "Reset bei hohem Takt aktivieren"},
      {0, 1, 1, 5, 9, true, "Reset wirkt nicht auf fallende Flanke"},
      {1, 1, 1, 5, 0, true, "Reset hat Vorrang vor Enable=1"},
  });

  int sim_time = 0;
  int passed = 0;
  const auto total = steps.size() - 1;  // Erster Schritt ohne Vergleich.

  for (const auto &step : steps) {
    dut.clk = step.clk;
    dut.rst = step.rst;
    dut.enable = step.enable;
    dut.d = step.d;

    // Nach jeder Signaländerung auswerten und den Zustand für Surfer speichern.
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
