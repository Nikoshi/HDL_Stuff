#include "Vcpu_v2_top.h"
#include "verilated_vcd_c.h"

#include <array>
#include <iostream>

// Unabhängiges Softwaremodell der festen CPU-v2-ISA.
// Die erwarteten Befehle stehen hier separat vom Spade-ROM.
constexpr std::array<int, 16> kProgram = {
    0x45, 0x0F, 0x95, 0x81,
    0xC0, 0x2F, 0x1A, 0x3F,
    0xC0, 0x80, 0xC0, 0xC0,
    0xC0, 0xC0, 0xC0, 0xC0
};

struct CpuV2Reference {
  int pc = 0;
  int acc = 0;
  bool initialized = false;

  int instruction() const {
    return kProgram[static_cast<unsigned>(pc)];
  }

  void rising_edge(bool rst, bool advance) {
    if (rst) {
      pc = 0;
      acc = 0;
      initialized = true;
      return;
    }
    if (!initialized || !advance) return;

    // Beide neuen Registerwerte werden aus demselben alten Zustand
    // berechnet. Insbesondere prüft JZ den gespeicherten alten ACC.
    const int old_acc = acc;
    const int word = instruction();
    const int opcode = (word >> 4) & 15;
    const int imm = word & 15;

    int next_pc = (pc + 1) & 15;
    int next_acc = old_acc;
    switch (opcode) {
      case 0x0: next_acc = (old_acc + imm) & 15; break;  // ADD
      case 0x1: next_acc = old_acc & imm; break;         // AND
      case 0x2: next_acc = old_acc | imm; break;         // OR
      case 0x3: next_acc = old_acc ^ imm; break;         // XOR
      case 0x4: next_acc = imm; break;                   // LDI
      case 0x8: next_pc = imm; break;                    // JMP
      case 0x9: if (old_acc == 0) next_pc = imm; break;  // JZ
      case 0xC: break;                                  // NOP
      default:
        // Das festgelegte Programm enthält keine reservierten Opcodes.
        // Sie würden in dieser Referenzmodell-Situation nichts ändern.
        break;
    }

    pc = next_pc;
    acc = next_acc;
  }
};

int main() {
  Verilated::traceEverOn(true);
  Vcpu_v2_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/cpu_v2.vcd");

  CpuV2Reference expected;
  int last_clock = 0;
  int sim_time = 0;
  int checks = 0;

  auto step = [&](int clk, int rst, int advance, const char* context) -> bool {
    dut.clk = clk;
    dut.rst = rst;
    dut.advance = advance;
    dut.eval();

    if (!last_clock && clk) {
      expected.rising_edge(rst != 0, advance != 0);
    }
    last_clock = clk;
    trace.dump(sim_time);
    sim_time += 10;

    // Ein Registerwert ist vor der ersten Reset-Flanke undefiniert.
    if (!expected.initialized) return true;

    if (int(dut.pc) != expected.pc ||
        int(dut.acc) != expected.acc ||
        int(dut.instruction) != expected.instruction()) {
      std::cerr << "CPU v2 falsch: " << context
                << " (clk=" << clk << ", rst=" << rst
                << ", advance=" << advance << ")\n"
                << "  erwartet (PC,ACC,instruction)=("
                << expected.pc << "," << expected.acc
                << ",0x" << std::hex << expected.instruction() << ")\n"
                << "  erhalten (PC,ACC,instruction)=("
                << std::dec << int(dut.pc) << "," << int(dut.acc)
                << ",0x" << std::hex << int(dut.instruction)
                << std::dec << ")\n";
      trace.close();
      dut.final();
      return false;
    }

    ++checks;
    return true;
  };

  auto tick = [&](int rst, int advance, const char* context) -> bool {
    // Änderungen ohne neue steigende Flanke dürfen PC/ACC nicht verändern.
    // Das ROM muss weiterhin zum aktuell gespeicherten PC passen.
    return step(0, rst, advance, "Vor steigender Flanke") &&
           step(1, rst, advance, context) &&
           step(1, !rst, !advance, "Steuerwechsel bei hohem Takt") &&
           step(0, !rst, !advance, "Fallende Flanke");
  };

  // Initialisierung mit synchronem Reset trotz advance=0.
  if (!tick(1, 0, "Initialer Reset")) return 1;

  // Drei vollständige Durchläufe des 20-Befehle-Programms.
  // Jeder dritte Versuch wird zusätzlich durch eine Pause unterbrochen.
  for (int cycle = 0; cycle < 60; ++cycle) {
    if (cycle % 3 == 0 &&
        !tick(0, 0, "Pause innerhalb des Programms")) return 1;
    if (!tick(0, 1, "Countdown und LED-Alarm")) return 1;

    if ((cycle + 1) % 20 == 0 &&
        (expected.pc != 0 || expected.acc != 5)) {
      std::cerr << "CPU-v2-Referenzprogramm kehrt nach 20 Schritten "
                   "nicht zu PC=0, ACC=5 zurück\n";
      trace.close();
      dut.final();
      return 1;
    }
  }

  // Reset in jeder Phase des Programms; anschließend Pausieren und
  // Weiterlaufen. Damit werden auch Reset und JZ-Zeitpunkte abgedeckt.
  for (int prefix = 0; prefix < 20; ++prefix) {
    if (!tick(1, prefix & 1, "Reset vor Teilablauf")) return 1;
    for (int i = 0; i < prefix; ++i) {
      if (!tick(0, 1, "Teilablauf vor Reset")) return 1;
    }
    if (!tick(1, 0, "Reset mit deaktiviertem advance") ||
        !tick(0, 0, "Pause nach Reset")) return 1;
    for (int i = 0; i < 4; ++i) {
      if (!tick(0, 1, "Neustart nach Reset")) return 1;
    }
  }

  // Längere deterministische Mischung aus Reset, Pausen und Schritten.
  for (int i = 0; i < 512; ++i) {
    if (!tick(i % 67 == 0, i % 7 != 0, "Gemischte Ablaufsequenz")) {
      return 1;
    }
  }

  trace.close();
  dut.final();

  // 1 initialer Reset, 60 Programmschritte + 20 Pausen,
  // 20 Testphasen mit je (7 + prefix) Takten und 512 gemischte Takte.
  // Jeder Tick hat vier Beobachtungen; die erste vor dem Reset zählt nicht.
  constexpr int expected_checks =
      (1 + 60 + 20 + 20 * 7 + (19 * 20 / 2) + 512) * 4 - 1;
  if (checks != expected_checks) {
    std::cerr << "CPU-v2-Testbenchfehler: " << checks
              << "/" << expected_checks << " Vergleiche\n";
    return 1;
  }

  std::cout << "CPU v2 integriert: " << checks
            << "/" << expected_checks << " Tests bestanden\n";
  return 0;
}
