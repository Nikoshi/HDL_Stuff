#include "Vdecoder8_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

// Der Decoder soll nur zwei 4-Bit-Felder eines 8-Bit-Worts ausgeben.
// Die Testbench interpretiert die Opcodes absichtlich noch nicht.
int main() {
  Verilated::traceEverOn(true);
  Vdecoder8_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/decoder8.vcd");

  int checked = 0;
  int sim_time = 0;
  auto check = [&](int instruction, const char* context) -> bool {
    dut.instruction = instruction & 0xFF;
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    // Unabhaengiges Referenzmodell fuer die beiden Nibbles.
    const int expected_opcode = (instruction >> 4) & 0xF;
    const int expected_immediate = instruction & 0xF;

    if (int(dut.opcode) != expected_opcode ||
        int(dut.immediate) != expected_immediate) {
      std::cerr << "8-Bit-Decoder falsch: " << context
                << ", instruction=" << instruction << "\n"
                << "  erwartet opcode=" << expected_opcode
                << ", immediate=" << expected_immediate << "\n"
                << "  erhalten opcode=" << int(dut.opcode)
                << ", immediate=" << int(dut.immediate) << "\n";
      trace.close();
      dut.final();
      return false;
    }

    ++checked;
    return true;
  };

  // Saemtliche 256 Instruktionsbits, auch reservierte Opcodes.
  // Ein Felddecoder darf diese Bits nicht willkuerlich veraendern.
  for (int instruction = 0; instruction < 256; ++instruction) {
    if (!check(instruction, "Alle 256 Befehlswoerter")) return 1;
  }

  // Schnelle, nicht-monotone Aenderungen ohne Clock pruefen.
  // 73 und 256 sind teilerfremd: die erzeugte Sequenz ist permutiert.
  for (int i = 0; i < 128; ++i) {
    const int word = (73 * i + 41) & 0xFF;
    if (!check(word, "Direkter Eingangssignalwechsel")) return 1;
  }

  trace.close();
  dut.final();

  constexpr int expected_checks = 256 + 128;
  if (checked != expected_checks) {
    std::cerr << "Decoder8-Testbench-Fehler: " << checked
              << "/" << expected_checks << "\n";
    return 1;
  }

  std::cout << "8-Bit-Decoder: " << checked << "/"
            << expected_checks << " Tests bestanden\n";
  return 0;
}
