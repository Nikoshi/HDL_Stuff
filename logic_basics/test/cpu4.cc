#include "Vcpu4_top.h"
#include "instruction6_model.hpp"
#include "verilated_vcd_c.h"

#include <array>
#include <iostream>

// Festes Referenzprogramm aus der ROM-Spezifikation, nicht aus der RTL-Ausgabe.
constexpr std::array<int, 16> kProgram = {
    0x25, 0x0B, 0x37, 0x13,
    0x01, 0x3F, 0x28, 0x16,
    0x0E, 0x32, 0x24, 0x03,
    0x19, 0x3B, 0x22, 0x05
};

// Software-CPU: Der ACC verarbeitet ROM[PC_alt], erst dann aendert sich PC.
struct Cpu4Reference {
  int pc = 0;
  bool initialized = false;
  Instruction6Model execute;

  void rising_edge(bool rst, bool advance, bool jump, int target) {
    if (rst) {
      pc = 0;
      initialized = true;
      execute.rising_edge(true, false, 0);
      return;
    }
    if (!initialized) return;

    // Beide Register erhalten Werte des alten CPU-Zustands.
    const int old_instruction = kProgram[static_cast<unsigned>(pc)];
    execute.rising_edge(false, advance, old_instruction);

    if (advance) {
      pc = jump ? (target & 0xF) : ((pc + 1) & 0xF);
    }
  }

  int instruction() const {
    return kProgram[static_cast<unsigned>(pc)];
  }
};

int main() {
  Verilated::traceEverOn(true);
  Vcpu4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/cpu4.vcd");

  Cpu4Reference reference;
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

    // Vor der ersten Reset-Flanke sind PC und ACC nicht definiert.
    if (!reference.initialized) return true;

    const int expected_pc = reference.pc;
    const int expected_instruction = reference.instruction();
    const int expected_acc = reference.execute.acc;

    if (int(dut.pc) != expected_pc ||
        int(dut.instruction) != expected_instruction ||
        int(dut.acc) != expected_acc) {
      std::cerr << "CPU4 falsch: " << context
                << " (clk=" << clk << ", rst=" << rst
                << ", advance=" << advance << ", jump=" << jump
                << ", target=" << (target & 0xF) << ")\n"
                << "  erwartet (PC,instruction,ACC)=("
                << expected_pc << "," << expected_instruction << ","
                << expected_acc << ")\n"
                << "  erhalten (PC,instruction,ACC)=("
                << int(dut.pc) << "," << int(dut.instruction) << ","
                << int(dut.acc) << ")\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checked;
    return true;
  };

  auto tick = [&](int rst, int advance, int jump, int target,
                  const char *context) -> bool {
    const int changed_target = (target + 5) & 0xF;
    return step(0, rst, advance, jump, target, "Vor steigender Flanke") &&
           step(1, rst, advance, jump, target, context) &&
           step(1, 0, !advance, !jump, changed_target,
                "Steuerwechsel ohne neue steigende Flanke") &&
           step(0, 0, !advance, !jump, changed_target, "Fallende Flanke");
  };

  // Initialer synchroner Reset: vor der Flanke bleibt der Zustand
  // unbestimmt, danach muessen beide Register 0 und ROM[0] sichtbar sein.
  if (!step(0, 1, 1, 1, 15, "Reset vorbereiten") ||
      !step(1, 1, 1, 1, 15, "Reset beider Register") ||
      !step(1, 0, 1, 1, 7, "Keine weitere Flanke bei hohem Takt") ||
      !step(0, 0, 1, 1, 7, "Fallende Flanke nach Reset")) {
    return 1;
  }

  // Zwei vollständige Programmdurchlaeufe inklusive Modulo-16-Ueberlauf.
  for (int i = 0; i < 32; ++i) {
    if (!tick(0, 1, 0, 0, "Sequentielles Programm")) return 1;
  }

  // Jede Ausgangsadresse gegen jedes Sprungziel; dabei wird auch
  // geprueft, dass ein Jump den *alten* Befehl noch ausfuehrt.
  for (int from = 0; from < 16; ++from) {
    for (int target = 0; target < 16; ++target) {
      if (!tick(1, 1, 1, 15, "Reset vor Sprung") ||
          !tick(0, 1, 1, from, "Startadresse anwaehlen") ||
          !tick(0, 1, 1, target, "Alten Befehl ausfuehren und springen") ||
          !tick(0, 0, 1, (target + 7) & 0xF,
                "Sprung waehrend Pause ignorieren") ||
          !tick(0, 1, 0, 0, "Befehl am Sprungziel ausfuehren")) {
        return 1;
      }
    }
  }

  // Gemischte Testsequenz: Reset, Pausen und Ziele mit variierender
  // Adresse gegen den unabhaengigen Emulator pruefen.
  for (int i = 0; i < 256; ++i) {
    const int rst = (i % 61 == 0);
    const int advance = (i % 5 != 0);
    const int jump = (i % 7 == 0);
    const int target = (9 * i + 3) & 0xF;
    if (!tick(rst, advance, jump, target, "Gemischte CPU-Sequenz")) {
      return 1;
    }
  }

  if (!step(0, 1, 0, 1, 13, "Reset bei niedrigem Takt") ||
      !step(1, 1, 0, 1, 13, "Reset hat Vorrang trotz Pause und Sprung") ||
      !step(0, 0, 0, 1, 13, "Nach Reset keine weitere Aenderung")) {
    return 1;
  }

  trace.close();
  dut.final();

  constexpr int expected_checks = 3 + (32 * 4) + (16 * 16 * 5 * 4)
                                 + (256 * 4) + 3;
  if (checked != expected_checks) {
    std::cerr << "CPU4-Testbench-Fehler: " << checked << "/"
              << expected_checks << " Zustandsvergleiche\n";
    return 1;
  }

  std::cout << "4-Bit-CPU: " << checked << "/" << expected_checks
            << " Tests bestanden\n";
  return 0;
}
