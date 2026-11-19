#include "Vtraffic_light_top.h"
#include "verilated_vcd_c.h"

#include <array>
#include <iostream>
#include <vector>

struct TestStep {
  int clk, rst, advance;
  bool check_outputs;
  const char *description;
};

struct Lights {
  int red, yellow, green;
};

int main() {
  Verilated::traceEverOn(true);

  Vtraffic_light_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/traffic_light.vcd");

  // Die Reihenfolge entspricht dem gewünschten Ampel-Zyklus.
  // Rot-Gelb ist ein eigener Zustand mit zwei aktiven Lampen.
  const std::array<Lights, 4> phase_outputs = {{
      {1, 0, 0},  // Rot
      {1, 1, 0},  // Rot-Gelb
      {0, 0, 1},  // Grün
      {0, 1, 0},  // Gelb
  }};

  std::vector<TestStep> steps = {
      {0, 1, 0, false, "Synchronen Reset vorbereiten"},
      {1, 1, 0, true, "Nach Reset ist die Ampel rot"},
  };

  // Jede Teilphase kontrolliert steigende und fallende Flanken.
  // Auch Eingangsänderungen bei hohem Takt müssen wirkungslos bleiben.
  auto tick = [&](int reset, int advance, const char *description) {
    steps.push_back({0, reset, advance, true, "Fallende Flanke"});
    steps.push_back({1, reset, advance, true, description});
    steps.push_back({1, reset, 1 - advance, true,
                     "Advance ändert sich bei hohem Takt"});
    steps.push_back({0, reset, 1 - advance, true,
                     "Erneute fallende Flanke"});
  };

  // Rot bleibt bei deaktivierter Freigabe erhalten.
  for (int i = 0; i < 5; ++i) {
    tick(0, 0, "Ohne Advance im aktuellen Zustand bleiben");
  }

  // Drei vollständige Ampelrunden.
  for (int i = 0; i < 12; ++i) {
    tick(0, 1, "Nächster Ampelzustand");
  }

  // Gemischtes Muster aus Pausen und Zustandswechseln.
  for (int i = 0; i < 24; ++i) {
    tick(0, (i % 4) != 0, "Gemischtes Advance-Muster");
  }

  // Reset aus jeder der vier Phasen. Auch bei gleichzeitig aktivem
  // Advance muss Reset gewinnen, allerdings erst zur steigenden Flanke.
  for (int phase = 0; phase < 4; ++phase) {
    tick(1, 1, "Für nächsten Fall auf Rot zurücksetzen");
    for (int i = 0; i < phase; ++i) {
      tick(0, 1, "Zu prüfende Ausgangsphase einstellen");
    }
    steps.push_back({1, 0, 0, true, "Zustand ohne Advance halten"});
    steps.push_back({1, 1, phase % 2, true,
                     "Reset wird bei hohem Takt aktiviert"});
    steps.push_back({0, 1, phase % 2, true,
                     "Reset verändert bei fallender Flanke nichts"});
    steps.push_back({1, 1, phase % 2, true,
                     "Reset muss zu Rot wechseln"});
  }

  // Nach Reset bei ausgeschaltetem Advance auf Rot bleiben.
  for (int i = 0; i < 5; ++i) {
    tick(0, 0, "Nach Reset weiterhin Rot");
  }

  int current_phase = 0;
  int previous_clk = 0;
  int sim_time = 0;
  int passed = 0;
  const auto total = steps.size() - 1;

  for (const auto &step : steps) {
    dut.clk = step.clk;
    dut.rst = step.rst;
    dut.advance = step.advance;
    dut.eval();

    // Unabhängiges Referenzmodell des Zustandsübergangs.
    if (previous_clk == 0 && step.clk == 1) {
      if (step.rst) {
        current_phase = 0;
      } else if (step.advance) {
        current_phase = (current_phase + 1) % 4;
      }
    }

    trace.dump(sim_time);
    sim_time += 10;
    previous_clk = step.clk;

    if (!step.check_outputs) {
      continue;
    }

    const auto &expected = phase_outputs[current_phase];
    if (int(dut.red) != expected.red ||
        int(dut.yellow) != expected.yellow ||
        int(dut.green) != expected.green) {
      std::cerr << "Ampel-FSM falsch: " << step.description
                << " (clk=" << step.clk << ", rst=" << step.rst
                << ", advance=" << step.advance
                << ", Phase=" << current_phase
                << "): erwartet (R,Y,G)=(" << expected.red << ","
                << expected.yellow << "," << expected.green
                << "), erhalten=(" << int(dut.red) << ","
                << int(dut.yellow) << "," << int(dut.green)
                << ")\n";
      trace.close();
      dut.final();
      return 1;
    }

    ++passed;
  }

  dut.final();
  trace.close();

  std::cout << "Ampel-FSM: " << passed << "/" << total
            << " Tests bestanden\n";
  return 0;
}
