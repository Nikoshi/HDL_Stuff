#include "Vcpu_v2_state_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

// Unabhängiges Registermodell. Alle Berechnungen benutzen die alten
// Zustände, auch wenn PC und ACC an derselben Taktflanke wechseln.
struct StateReference {
  int pc = 0;
  int acc = 0;
  bool initialized = false;

  void rising_edge(bool rst, bool advance, int immediate,
                   int alu_result, bool acc_write,
                   bool acc_from_imm, int pc_mode) {
    if (rst) {
      pc = 0;
      acc = 0;
      initialized = true;
      return;
    }
    if (!initialized || !advance) return;

    const int old_pc = pc;
    const int old_acc = acc;
    const int imm = immediate & 0xF;

    // pc_mode=3 ist bewusst als sichere sequentielle Fortsetzung definiert.
    int next_pc = (old_pc + 1) & 0xF;
    if (pc_mode == 1 || (pc_mode == 2 && old_acc == 0)) {
      next_pc = imm;
    }
    const int next_acc =
        acc_write ? (acc_from_imm ? imm : (alu_result & 0xF)) : old_acc;
    pc = next_pc;
    acc = next_acc;
  }
};

int main() {
  Verilated::traceEverOn(true);
  Vcpu_v2_state_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/state_v2.vcd");

  StateReference expected;
  int last_clock = 0;
  int sim_time = 0;
  int checks = 0;

  auto drive = [&](int clk, int rst, int advance, int immediate,
                   int alu_result, int acc_write, int acc_from_imm,
                   int pc_mode, const char* context) -> bool {
    dut.clk = clk;
    dut.rst = rst;
    dut.advance = advance;
    dut.immediate = immediate & 0xF;
    dut.alu_result = alu_result & 0xF;
    dut.acc_write = acc_write;
    dut.acc_from_imm = acc_from_imm;
    dut.pc_mode = pc_mode & 3;
    dut.eval();

    if (!last_clock && clk) {
      expected.rising_edge(rst, advance, immediate, alu_result,
                           acc_write, acc_from_imm, pc_mode & 3);
    }
    last_clock = clk;
    trace.dump(sim_time);
    sim_time += 10;

    if (!expected.initialized) return true;
    if (int(dut.pc) != expected.pc || int(dut.acc) != expected.acc) {
      std::cerr << "CPU-v2-Zustandslogik falsch: " << context
                << " (clk=" << clk << ", rst=" << rst
                << ", advance=" << advance << ", immediate="
                << (immediate & 0xF) << ", alu_result="
                << (alu_result & 0xF) << ", acc_write="
                << acc_write << ", acc_from_imm=" << acc_from_imm
                << ", pc_mode=" << (pc_mode & 3) << ")\n"
                << "  erwartet: pc=" << expected.pc
                << ", acc=" << expected.acc << "\n"
                << "  erhalten: pc=" << int(dut.pc)
                << ", acc=" << int(dut.acc) << "\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checks;
    return true;
  };

  auto tick = [&](int rst, int advance, int imm, int alu,
                  int write, int from_imm, int mode,
                  const char* context) -> bool {
    // Keine Flanke: alle Ausgänge müssen unverändert bleiben.
    // Ebenso auf fallender Flanke und bei Eingangswechsel bei clk=1.
    return drive(0, rst, advance, imm, alu, write, from_imm, mode, context)
        && drive(1, rst, advance, imm, alu, write, from_imm, mode, context)
        && drive(1, !rst, !advance, (imm + 5) & 0xF,
                 (alu + 9) & 0xF, !write, !from_imm, (mode + 1) & 3,
                 "Eingänge ohne neue Taktflanke wechseln")
        && drive(0, !rst, !advance, (imm + 5) & 0xF,
                 (alu + 9) & 0xF, !write, !from_imm, (mode + 1) & 3,
                 "Fallende Taktflanke");
  };

  // Der synchrone Reset wirkt auch bei angehaltener CPU.
  if (!tick(1, 0, 15, 15, 1, 1, 1, "Initialer Reset")) return 1;

  // Alle möglichen Anfangszustände für PC und ACC gegen jeden
  // PC-Modus und beide ACC-Steuerbits prüfen.
  for (int initial_pc = 0; initial_pc < 16; ++initial_pc) {
    for (int initial_acc = 0; initial_acc < 16; ++initial_acc) {
      if (!tick(1, 1, 7, 8, 1, 0, 2, "Reset vor Testfall") ||
          !tick(0, 1, initial_acc, (initial_acc + 1) & 15, 1, 1, 0,
                "ACC-Anfangszustand setzen") ||
          !tick(0, 1, initial_pc, 11, 0, 0, 1,
                "PC-Anfangszustand setzen")) return 1;

      for (int mode = 0; mode < 4; ++mode) {
        for (int write = 0; write < 2; ++write) {
          for (int from_imm = 0; from_imm < 2; ++from_imm) {
            // Nach jedem getesteten Schritt den Anfangszustand
            // ohne versteckte DUT-Zugriffe über die Ports wiederherstellen.
            if (!tick(1, 0, 0, 0, 0, 0, 0, "Rücksetzen") ||
                !tick(0, 1, initial_acc, 0, 1, 1, 0,
                      "ACC setzen") ||
                !tick(0, 1, initial_pc, 0, 0, 0, 1,
                      "PC setzen")) return 1;

            const int immediate = (initial_pc * 7 + initial_acc * 3
                                  + mode * 5 + write) & 15;
            const int alu_result = (immediate + 9) & 15;
            if (!tick(0, 1, immediate, alu_result, write, from_imm,
                      mode, "Zustandsübergang")) return 1;
          }
        }
      }
    }
  }

  // Mehrere aufeinanderfolgende Schritte ohne Reset und Pausen
  // decken Fehler bei der Übernahme von Folgezuständen auf.
  for (int i = 0; i < 256; ++i) {
    const int imm = (13 * i + 5) & 15;
    if (!tick((i % 61 == 0), (i % 7 != 0), imm,
              (imm + 11) & 15, (i % 3 != 0), (i % 4 == 0),
              i & 3, "Gemischte Ablaufsequenz")) return 1;
  }

  trace.close();
  dut.final();

  // Initialer Reset: 1 Tick, je Testfall 3 + 4*2*2*4 Ticks,
  // gemischte Sequenz: 256 Ticks. Jeder Tick liefert 4 Vergleiche.
  constexpr int expected_checks = (1 + 16 * 16 * (3 + 4 * 2 * 2 * 4)
                                   + 256) * 4;
  if (checks != expected_checks) {
    std::cerr << "Zustandstest interne Anzahl falsch: "
              << checks << " statt " << expected_checks << "\n";
    return 1;
  }
  std::cout << "CPU-v2-Zustandslogik: " << checks
            << "/" << expected_checks << " Tests bestanden\n";
  return 0;
}
