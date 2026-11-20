#include "Vpc4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

// Unabhängiges Softwaremodell des PC: kein Spade oder RTL als Referenz.
struct PcReference {
  int address = 0;
  bool initialized = false;

  void rising_edge(bool rst, bool advance, bool jump, int target) {
    if (rst) {
      address = 0;
      initialized = true;
    } else if (initialized && advance) {
      address = jump ? (target & 0xF) : ((address + 1) & 0xF);
    }
  }
};

int main() {
  Verilated::traceEverOn(true);
  Vpc4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/pc4.vcd");

  PcReference reference;
  int last_clk = 0;
  int sim_time = 0;
  int checked = 0;

  auto step = [&](int clk, int rst, int advance, int jump, int target,
                  const char *context) -> bool {
    dut.clk = clk;
    dut.rst = rst;
    dut.advance = advance;
    dut.jump = jump;
    dut.target = target & 0xF;
    dut.eval();

    if (last_clk == 0 && clk == 1) {
      reference.rising_edge(rst != 0, advance != 0, jump != 0, target);
    }
    last_clk = clk;

    trace.dump(sim_time);
    sim_time += 10;

    // Vor der ersten Reset-Flanke ist der Registerinhalt undefiniert.
    if (!reference.initialized) return true;

    if (int(dut.pc) != reference.address) {
      std::cerr << "PC4 falsch: " << context
                << " (clk=" << clk << ", rst=" << rst
                << ", advance=" << advance << ", jump=" << jump
                << ", target=" << (target & 0xF)
                << "): erwartet PC=" << reference.address
                << ", erhalten=" << int(dut.pc) << "\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checked;
    return true;
  };

  auto tick = [&](int rst, int advance, int jump, int target,
                  const char *context) -> bool {
    const int changed_target = (target + 7) & 0xF;
    return step(0, rst, advance, jump, target, "Vor steigender Flanke") &&
           step(1, rst, advance, jump, target, context) &&
           step(1, 0, !advance, !jump, changed_target,
                "Änderungen bei hohem Takt") &&
           step(0, 0, !advance, !jump, changed_target,
                "Fallende Flanke");
  };

  // Reset hat Vorrang selbst dann, wenn Zählen und Springen aktiv sind.
  if (!step(0, 1, 1, 1, 15, "Reset vorbereiten") ||
      !step(1, 1, 1, 1, 15, "Synchroner Reset") ||
      !step(1, 0, 1, 1, 7, "Ohne neue Flanke unverändert") ||
      !step(0, 0, 1, 1, 7, "Nach Reset fallende Flanke")) {
    return 1;
  }

  // Alle 16 Ausgangsadressen gegen alle 16 möglichen Sprungziele.
  // Startadresse durch deterministisches Vorwärtszählen herstellen.
  for (int address = 0; address < 16; ++address) {
    for (int target = 0; target < 16; ++target) {
      if (!tick(1, 1, 1, target, "Reset vor jeder Kombination")) return 1;
      for (int i = 0; i < address; ++i) {
        if (!tick(0, 1, 0, 0, "Startadresse einstellen")) return 1;
      }
      if (!tick(0, 1, 1, target, "Absoluter Sprung bei advance=1") ||
          !tick(0, 0, 1, (target + 7) & 0xF,
                "Sprung bei advance=0 ignorieren") ||
          !tick(0, 1, 0, (target + 9) & 0xF,
                "Nach Sprung sequenziell weiterzählen")) {
        return 1;
      }
    }
  }

  // Längere Folge mit Pausen, Sprüngen, Überlauf und Reset.
  for (int i = 0; i < 96; ++i) {
    const int rst = (i % 23 == 0);
    const int advance = (i % 5 != 0);
    const int jump = (i % 7 == 0);
    const int target = (9 * i + 5) & 0xF;
    if (!tick(rst, advance, jump, target, "Gemischte PC-Steuerung")) return 1;
  }

  if (!step(0, 1, 0, 1, 7, "Reset ohne Flanke") ||
      !step(1, 1, 0, 1, 7, "Reset vor Pause und Sprung") ||
      !step(0, 0, 0, 1, 7, "Nach Reset unverändert")) {
    return 1;
  }

  constexpr int expected_checks =
      3 + (16 * (16 * 4 + 120) * 4) + (96 * 4) + 3;
  if (checked != expected_checks) {
    std::cerr << "PC4-Testbench-Fehler: " << checked << "/"
              << expected_checks << " Zustandsprüfungen\n";
    trace.close();
    dut.final();
    return 1;
  }

  trace.close();
  dut.final();
  std::cout << "4-Bit-Programmzaehler: " << checked
            << "/" << expected_checks << " Tests bestanden\n";
  return 0;
}
