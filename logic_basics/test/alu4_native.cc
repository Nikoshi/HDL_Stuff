#include "Valu4_native_top.h"
#include "verilated_vcd_c.h"

#include <iostream>
#include <string>

struct Expected {
  int result;
  bool carry;
  bool zero;
};

Expected expected_for(int a, int b, int op) {
  // Referenzmodell der ALU, bewusst unabhängig von Spade implementiert.
  int value = 0;
  bool carry = false;

  switch (op) {
    case 0: {
      const int full_sum = a + b;
      value = full_sum & 0xF;
      carry = (full_sum & 0x10) != 0;
      break;
    }
    case 1:
      value = a & b;
      break;
    case 2:
      value = a | b;
      break;
    case 3:
      value = a ^ b;
      break;
  }

  return {value, carry, value == 0};
}

int main() {
  Verilated::traceEverOn(true);

  Valu4_native_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/alu4_native.vcd");

  int passed = 0;
  int sim_time = 0;
  auto check = [&](int a, int b, int op, const std::string &description) {
    dut.a = a;
    dut.b = b;
    dut.op = op;
    dut.eval();

    trace.dump(sim_time);
    sim_time += 10;

    const auto expected = expected_for(a, b, op);
    if (int(dut.result) != expected.result ||
        bool(dut.carry_out) != expected.carry ||
        bool(dut.zero) != expected.zero) {
      std::cerr << "4-Bit-ALU (nativ) falsch: " << description
                << " (op=" << op << ", a=" << a << ", b=" << b
                << "): erwartet (result,carry,zero)=("
                << expected.result << "," << expected.carry << ","
                << expected.zero << "), erhalten=("
                << int(dut.result) << "," << int(dut.carry_out)
                << "," << int(dut.zero) << ")\n";
      trace.close();
      dut.final();
      return false;
    }
    ++passed;
    return true;
  };

  // Exhaustive Wahrheitstabelle: 4 Operationen * 16 * 16 Paare.
  // Die Verteilung der Operanden deckt Null, volle Einsen und Carry ab.
  for (int op = 0; op < 4; ++op) {
    for (int a = 0; a < 16; ++a) {
      for (int b = 0; b < 16; ++b) {
        if (!check(a, b, op, "Alle vierbittigen Operandenpaare")) {
          return 1;
        }
      }
    }
  }

  // Bei gleichem Operandenpaar wird die Operation mehrfach gewechselt.
  // Das deckt fehlerhaft gespeicherte oder nicht ausgewählte Resultate auf.
  for (int i = 0; i < 24; ++i) {
    const int a = (i * 11 + 5) & 0xF;
    const int b = (i * 7 + 3) & 0xF;
    const int op = (i * 3 + 1) % 4;
    if (!check(a, b, op, "Direkter Wechsel von Operation und Operanden")) {
      return 1;
    }
  }

  dut.final();
  trace.close();
  std::cout << "4-Bit-ALU (nativ): " << passed << "/1048 Tests bestanden\n";
  return 0;
}
