#include "Vinstruction_exec6_top.h"
#include "instruction6_model.hpp"
#include "verilated_vcd_c.h"

#include <iostream>

int main() {
  Verilated::traceEverOn(true);
  Vinstruction_exec6_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/instruction_exec6.vcd");

  Instruction6Model reference;
  int last_clk = 0;
  int sim_time = 0;
  int checked = 0;

  auto step = [&](int clk, int rst, int enable, int instruction,
                  const char *context) -> bool {
    dut.clk = clk;
    dut.rst = rst;
    dut.enable = enable;
    dut.instruction = instruction & 0x3F;
    dut.eval();

    if (last_clk == 0 && clk == 1) {
      reference.rising_edge(rst != 0, enable != 0, instruction & 0x3F);
    }
    last_clk = clk;

    trace.dump(sim_time);
    sim_time += 10;

    // Vor der ersten Reset-Flanke hat das Hardware-Register keinen
    // spezifizierten Wert. Erst danach vergleichen wir ACC mit q.
    if (!reference.initialized) return true;

    if (int(dut.q) != reference.acc) {
      std::cerr << "Befehlsausführung falsch: " << context
                << " (clk=" << clk << ", rst=" << rst
                << ", enable=" << enable
                << ", instruction=" << (instruction & 0x3F)
                << "): erwartet ACC=" << reference.acc
                << ", erhalten q=" << int(dut.q) << "\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checked;
    return true;
  };

  // Ein Takt mit anschließenden Signalwechseln ohne aktive Flanke.
  // Diese dürfen den gespeicherten Akkumulator nicht verändern.
  auto tick = [&](int rst, int enable, int instruction,
                  const char *context) -> bool {
    const int changed_instruction = (instruction ^ 0x3F) & 0x3F;
    return step(0, rst, enable, instruction, "Vor der steigenden Flanke") &&
           step(1, rst, enable, instruction, context) &&
           step(1, 0, !enable, changed_instruction, "Befehl wechselt bei hohem Takt") &&
           step(0, 0, !enable, changed_instruction, "Fallende Flanke");
  };

  // Reset hat auch bei enable=true Vorrang, und ist synchron.
  if (!step(0, 1, 1, 0x3F, "Reset vorbereiten") ||
      !step(1, 1, 1, 0x3F, "Synchroner Reset") ||
      !step(1, 0, 1, 0x0F, "Kein weiteres Update bei hohem Takt") ||
      !step(0, 0, 1, 0x0F, "Fallende Flanke")) {
    return 1;
  }

  // Alle 64 Maschinenbefehle gegen alle 16 moeglichen ACC-Ausgangswerte:
  // Mit OR a wird der Anfangswert deterministisch hergestellt.
  for (int a = 0; a < 16; ++a) {
    for (int instruction = 0; instruction < 64; ++instruction) {
      if (!tick(1, 1, 0x3F, "Reset zwischen Befehlsfolgen") ||
          !tick(0, 1, 0x20 | a, "ACC mit OR-Immediate vorbereiten") ||
          !tick(0, 1, instruction, "Maschinenbefehl ausführen") ||
          !tick(0, 0, instruction ^ 0x3F, "Ohne Enable halten")) {
        return 1;
      }
    }
  }

  // Laengere Folge ohne Reset: Ein Emulator muss auch Folgezustaende
  // schrittweise korrekt vorhersagen, nicht nur einzelne Operationen.
  for (int i = 0; i < 128; ++i) {
    if (!tick(0, 1, (13 * i + 7) & 0x3F, "Aufeinanderfolgende Befehle")) {
      return 1;
    }
  }

  if (!step(0, 1, 0, 0x11, "Reset bei niedrigem Takt setzen") ||
      !step(1, 1, 0, 0x11, "Reset wirkt trotz ausgeschaltetem Enable") ||
      !step(0, 0, 0, 0x11, "Nach Reset halten")) {
    return 1;
  }

  constexpr int expected_checks = 3 + (16 * 64 * 4 * 4) + (128 * 4) + 3;
  if (checked != expected_checks) {
    std::cerr << "Testbench-Fehler: " << checked << "/"
              << expected_checks << " Pruefungen durchlaufen\n";
    trace.close();
    dut.final();
    return 1;
  }

  trace.close();
  dut.final();
  std::cout << "6-Bit-Befehlsausfuehrung: " << checked
            << "/" << expected_checks << " Tests bestanden\n";
  return 0;
}
