#include "Vdecoder6_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

int main() {
  Verilated::traceEverOn(true);
  Vdecoder6_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/decoder6.vcd");

  int checked = 0;
  int sim_time = 0;
  auto check = [&](int instruction, const char *description) -> bool {
    dut.instruction = instruction & 0x3F;
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    // Unabhaengiges Referenzmodell fuer das verbindliche Befehlsformat:
    // Bits [5:4] = op, Bits [3:0] = unmittelbarer Operand.
    const int expected_op = (instruction >> 4) & 0x3;
    const int expected_operand = instruction & 0xF;

    if (int(dut.op) != expected_op || int(dut.operand) != expected_operand) {
      std::cerr << "6-Bit-Decoder falsch: " << description
                << " (instruction=" << instruction << ")"
                << ": erwartet (op,operand)=(" << expected_op
                << "," << expected_operand << "), erhalten=("
                << int(dut.op) << "," << int(dut.operand) << ")\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checked;
    return true;
  };

  // Saemtliche 64 moeglichen sechs Bit breiten Befehle.
  for (int instruction = 0; instruction < 64; ++instruction) {
    if (!check(instruction, "Vollstaendige Befehls-Wahrheitstabelle")) {
      return 1;
    }
  }

  // Ohne Takt darf der Decoder keinerlei Zustand speichern.
  // Jeder direkte Eingangswechsel muss sich im Ausgang widerspiegeln.
  for (int i = 0; i < 32; ++i) {
    if (!check((17 * i + 9) & 0x3F, "Direkter Befehlswechsel")) {
      return 1;
    }
  }

  trace.close();
  dut.final();
  constexpr int expected_checks = 64 + 32;
  if (checked != expected_checks) {
    std::cerr << "Interner Testbench-Fehler: " << checked
              << " von " << expected_checks << " Pruefungen\n";
    return 1;
  }
  std::cout << "6-Bit-Decoder: " << checked
            << "/" << expected_checks << " Tests bestanden\n";
  return 0;
}
