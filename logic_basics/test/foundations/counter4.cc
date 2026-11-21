#include "Vcounter4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>
#include <vector>

struct TestStep {
  int clk, rst, enable;
  int expected_q;
  bool check_q;
  const char *description;
};

int main() {
  // Signalverläufe müssen vor dem Erzeugen des Simulationsmodells aktiviert werden.
  Verilated::traceEverOn(true);

  Vcounter4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/counter4.vcd");

  // Die Testbench startet mit einem synchronen Reset. Vor der ersten
  // steigenden Flanke ist der Speicherzustand noch nicht definiert.
  std::vector<TestStep> steps = {
      {0, 1, 0, 0, false, "Reset vorbereiten"},
      {1, 1, 0, 0, true, "Synchroner Reset bei deaktiviertem Enable"},
  };

  // Vollständige Zählfolge mit mindestens einem Überlauf und weiteren
  // Schritten nach dem Überlauf. Auch Zwischenzustände werden geprüft.
  for (int tick = 1; tick <= 20; ++tick) {
    const int before = (tick - 1) % 16;
    const int after = tick % 16;

    steps.push_back({1, 0, 1, before, true, "Enable ohne neue Flanke"});
    steps.push_back({0, 0, 1, before, true, "Fallende Taktflanke"});
    steps.push_back({1, 0, 1, after, true, "Steigende Taktflanke zaehlt"});
  }

  // Enable=0: Auch viele weitere steigende Taktflanken dürfen die
  // zuletzt gespeicherte Zahl 4 nicht verändern.
  for (int tick = 0; tick < 16; ++tick) {
    steps.push_back({1, 0, 0, 4, true, "Enable deaktivieren"});
    steps.push_back({0, 0, 0, 4, true, "Fallende Flanke bei Enable=0"});
    steps.push_back({1, 0, 0, 4, true, "Steigende Flanke bei Enable=0"});
  }

  // Enable wieder aktivieren und einen weiteren vollen Umlauf prüfen.
  for (int tick = 1; tick <= 16; ++tick) {
    const int before = (4 + tick - 1) % 16;
    const int after = (4 + tick) % 16;

    steps.push_back({1, 0, 1, before, true, "Enable wieder aktivieren"});
    steps.push_back({0, 0, 1, before, true, "Fallende Flanke nach Enable"});
    steps.push_back({1, 0, 1, after, true, "Zaehlen nach Pause"});
  }

  // Reset ist synchron und hat Vorrang vor Enable: Der aktuelle Wert 4
  // bleibt bis zur steigenden Flanke stehen, danach wird q gleich 0.
  steps.insert(steps.end(), {
      {1, 1, 0, 4, true, "Reset bei hohem Takt aktivieren"},
      {0, 1, 0, 4, true, "Fallende Flanke mit Reset"},
      {1, 1, 0, 0, true, "Reset trotz Enable=0"},
      {1, 0, 1, 0, true, "Reset deaktivieren ohne Flanke"},
      {0, 0, 1, 0, true, "Fallende Flanke nach Reset"},
      {1, 0, 1, 1, true, "Erster Zaehlschritt nach Reset"},
      {1, 1, 1, 1, true, "Reset bei Enable=1 ohne Flanke"},
      {0, 1, 1, 1, true, "Reset bei fallender Flanke"},
      {1, 1, 1, 0, true, "Reset hat Vorrang vor Enable=1"},
  });

  int sim_time = 0;
  int passed = 0;
  const auto total = steps.size() - 1;  // Initialisierung wird nicht verglichen.

  for (const auto &step : steps) {
    dut.clk = step.clk;
    dut.rst = step.rst;
    dut.enable = step.enable;

    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    if (!step.check_q) {
      continue;
    }

    if (dut.q != step.expected_q) {
      std::cerr << "4-Bit-Zaehler falsch: " << step.description
                << " (clk=" << step.clk
                << ", rst=" << step.rst
                << ", enable=" << step.enable
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

  std::cout << "4-Bit-Zaehler: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
