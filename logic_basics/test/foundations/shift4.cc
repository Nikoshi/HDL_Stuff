#include "Vshift4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>
#include <vector>

struct TestStep {
  int clk, rst, enable, serial_in;
  bool check_q;
  const char *description;
};

int main() {
  // Die Signalverläufe für Surfer vor der Modellinstanziierung aktivieren.
  Verilated::traceEverOn(true);

  Vshift4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/shift4.vcd");

  std::vector<TestStep> steps = {
      {0, 1, 0, 0, false, "Synchronen Reset vorbereiten"},
      {1, 1, 0, 0, true, "Reset bei steigender Flanke"},
  };

  // Ein vollständiger Schreibzyklus prüft auch Änderungen ohne Taktflanke.
  // Diese dürfen den gespeicherten Ausgang nicht beeinflussen.
  auto add_cycle = [&](int rst, int enable, int serial_in,
                       const char *description) {
    steps.push_back({0, rst, enable, serial_in, true, "Takt ist niedrig"});
    steps.push_back({1, rst, enable, serial_in, true, description});
    steps.push_back({1, rst, enable, 1 - serial_in, true,
                     "Eingang wechselt bei hohem Takt"});
    steps.push_back({0, rst, enable, 1 - serial_in, true,
                     "Fallende Flanke darf nicht schieben"});
  };

  // Die bekannten ersten vier Bits ergeben nacheinander 1, 2, 5 und 11.
  add_cycle(0, 1, 1, "Erstes Bit einschieben");
  add_cycle(0, 1, 0, "Zweites Bit einschieben");
  add_cycle(0, 1, 1, "Drittes Bit einschieben");
  add_cycle(0, 1, 1, "Viertes Bit einschieben");

  // Enable=0: Unterschiedliche Eingangssignale dürfen keine Bits verschieben.
  for (int i = 0; i < 8; ++i) {
    add_cycle(0, 0, i % 2, "Bei gesperrtem Enable Zustand halten");
  }

  // Beim nächsten aktiven Takt muss das älteste Bit herausfallen.
  add_cycle(0, 1, 0, "Herausschieben des hoechstwertigen Bits");

  // Gemischte serielle Daten mit gelegentlich deaktiviertem Enable prüfen.
  for (int i = 0; i < 24; ++i) {
    add_cycle(0, i % 5 != 0, ((i * 7 + 3) / 5) % 2,
              "Gemischte Datenfolge");
  }

  // Reset wird bei bereits hohem Takt aktiviert: Es darf noch nichts passieren.
  // Erst die nächste steigende Flanke setzt q auch bei Enable=0 auf null.
  steps.insert(steps.end(), {
      {1, 0, 0, 1, true, "Steigende Flanke ohne Enable"},
      {1, 1, 0, 1, true, "Reset ohne Flanke aktivieren"},
      {0, 1, 0, 1, true, "Fallende Flanke mit Reset"},
      {1, 1, 0, 1, true, "Synchroner Reset trotz Enable=0"},
  });

  // Eins einschieben und durch vier Nullbits wieder herausschieben.
  add_cycle(0, 1, 1, "Nach Reset ein Bit einschieben");
  for (int i = 0; i < 4; ++i) {
    add_cycle(0, 1, 0, "Bit nach links schieben");
  }

  // Erneut einen gesetzten Zustand herstellen und erst bei hohem Takt
  // reset aktivieren. Bei aktivem Enable darf auch hier nur die Flanke löschen.
  add_cycle(0, 1, 1, "Wert vor zweitem Reset speichern");
  steps.insert(steps.end(), {
      {1, 0, 0, 1, true, "Bei Enable=0 den Wert halten"},
      {1, 1, 1, 1, true, "Reset ohne Flanke bei Enable=1"},
      {0, 1, 1, 1, true, "Fallende Flanke mit Reset und Enable"},
      {1, 1, 1, 1, true, "Reset hat Vorrang vor Enable=1"},
  });

  // Ein unabhängiges C++-Zustandsmodell berechnet den Sollwert:
  // links schieben, auf vier Bits begrenzen, serielles Bit rechts einsetzen.
  int expected_q = 0;
  int previous_clk = 0;
  int sim_time = 0;
  int passed = 0;
  const auto total = steps.size() - 1;

  for (const auto &step : steps) {
    dut.clk = step.clk;
    dut.rst = step.rst;
    dut.enable = step.enable;
    dut.serial_in = step.serial_in;
    dut.eval();

    if (!previous_clk && step.clk) {
      if (step.rst) {
        expected_q = 0;
      } else if (step.enable) {
        expected_q = ((expected_q << 1) & 0xF) | step.serial_in;
      }
    }

    trace.dump(sim_time);
    sim_time += 10;
    previous_clk = step.clk;

    if (!step.check_q) {
      continue;
    }

    if (int(dut.q) != expected_q) {
      std::cerr << "4-Bit-Schieberegister falsch: " << step.description
                << " (clk=" << step.clk
                << ", rst=" << step.rst
                << ", enable=" << step.enable
                << ", serial_in=" << step.serial_in
                << "): erwartet q=" << expected_q
                << ", erhalten q=" << int(dut.q) << '\n';
      trace.close();
      dut.final();
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();
  std::cout << "4-Bit-Schieberegister: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
