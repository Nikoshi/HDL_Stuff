#include "Vtest_control8_top.h"
#include "verilated_vcd_c.h"

#include <array>
#include <iostream>

// Die erwarteten Steuersignale stammen aus dem ISA-Vertrag,
// nicht aus den Spade-Ausgaengen oder dem implementierten Decoder.
struct Controls {
  int acc_write;
  int acc_from_imm;
  int alu_op;
  int pc_mode;
};

constexpr Controls kNop{0, 0, 0, 0};

// Opcode-Tabelle: ALU 0..3, LDI 4, JMP 8, JZ 9, NOP 12.
// Acht reservierte Opcodes haben bewusst sichere NOP-Steuersignale.
constexpr std::array<Controls, 16> kReference = {{
    {1, 0, 0, 0}, // 0x0: ADD
    {1, 0, 1, 0}, // 0x1: AND
    {1, 0, 2, 0}, // 0x2: OR
    {1, 0, 3, 0}, // 0x3: XOR
    {1, 1, 0, 0}, // 0x4: LDI
    kNop, kNop, kNop, // 0x5..0x7: reserviert
    {0, 0, 0, 1}, // 0x8: JMP
    {0, 0, 0, 2}, // 0x9: JZ
    kNop, kNop, // 0xA..0xB: reserviert
    kNop, // 0xC: NOP
    kNop, kNop, kNop // 0xD..0xF: reserviert
}};

int main() {
  Verilated::traceEverOn(true);
  Vtest_control8_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/control8.vcd");

  int sim_time = 0;
  int checks = 0;

  auto check = [&](int op, const char* context) -> bool {
    dut.opcode = op & 0xF;
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    const Controls& expected = kReference[static_cast<unsigned>(op) & 0xF];
    if (int(dut.acc_write) != expected.acc_write ||
        int(dut.acc_from_imm) != expected.acc_from_imm ||
        int(dut.alu_op) != expected.alu_op ||
        int(dut.pc_mode) != expected.pc_mode) {
      std::cerr << "CPU-v2-Control8 falsch: " << context
                << " opcode=" << (op & 0xF) << "\n"
                << "  erwartet (acc_write,acc_from_imm,alu_op,pc_mode)=("
                << expected.acc_write << "," << expected.acc_from_imm
                << "," << expected.alu_op << "," << expected.pc_mode
                << ")\n"
                << "  erhalten=(" << int(dut.acc_write) << ","
                << int(dut.acc_from_imm) << "," << int(dut.alu_op)
                << "," << int(dut.pc_mode) << ")\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checks;
    return true;
  };

  // Alle 16 Opcodes einschliesslich der reservierten Werte.
  for (int op = 0; op < 16; ++op) {
    if (!check(op, "Komplette Opcode-Tabelle")) return 1;
  }

  // 128 zusaetzliche Eingabewechsel, ohne Clock oder Register.
  // 5 ist teilerfremd zu 16; dadurch werden die Opcodes durchmischt.
  for (int i = 0; i < 128; ++i) {
    if (!check((5 * i + 11) & 0xF,
               "Unmittelbarer kombinatorischer Opcode-Wechsel")) return 1;
  }

  trace.close();
  dut.final();
  constexpr int expected_checks = 16 + 128;
  if (checks != expected_checks) {
    std::cerr << "Control8-Testbenchfehler: " << checks << "/"
              << expected_checks << "\n";
    return 1;
  }

  std::cout << "CPU-v2-Steuerdecoder: " << checks << "/"
            << expected_checks << " Tests bestanden\n";
  return 0;
}
