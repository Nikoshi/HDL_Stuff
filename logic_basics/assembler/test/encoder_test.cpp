#include "encoder.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>

// Erwartete Opcode-Bits aus der ISA, bewusst unabhängig von encode().
struct Case {
  assembler::Opcode opcode;
  unsigned nibble;
  const char* name;
};

constexpr std::array<Case, 8> kCases = {{
    {assembler::Opcode::ADD, 0x0, "ADD"},
    {assembler::Opcode::AND, 0x1, "AND"},
    {assembler::Opcode::OR,  0x2, "OR"},
    {assembler::Opcode::XOR, 0x3, "XOR"},
    {assembler::Opcode::LDI, 0x4, "LDI"},
    {assembler::Opcode::JMP, 0x8, "JMP"},
    {assembler::Opcode::JZ,  0x9, "JZ"},
    {assembler::Opcode::NOP, 0xC, "NOP"},
}};

int main() {
  int passed = 0;
  int errors_reported = 0;

  // Jeder definierte Opcode mit allen 16 erlaubten 4-Bit-Operanden.
  for (const auto& tc : kCases) {
    for (unsigned imm = 0; imm < 16; ++imm) {
      const unsigned expected = (tc.nibble << 4) | imm;
      try {
        const auto got = assembler::encode(tc.opcode, imm);
        if (got == expected) {
          ++passed;
        } else if (errors_reported++ < 6) {
          std::cerr << tc.name << " " << imm << ": erwartet 0x"
                    << std::hex << expected << ", erhalten 0x"
                    << unsigned(got) << std::dec << "\n";
        }
      } catch (const std::exception& e) {
        if (errors_reported++ < 6) {
          std::cerr << tc.name << " " << imm
                    << ": unerwartete Ausnahme: " << e.what() << "\n";
        }
      }
    }
  }

  // Zu große Werte dürfen nicht durch Maskieren unbemerkt überlaufen.
  for (unsigned invalid : std::array<unsigned, 3>{{16, 17, 256}}) {
    try {
      (void)assembler::encode(assembler::Opcode::LDI, invalid);
      if (errors_reported++ < 6) {
        std::cerr << "LDI " << invalid
                  << ": std::out_of_range erwartet\n";
      }
    } catch (const std::out_of_range&) {
      ++passed;
    } catch (const std::exception& e) {
      if (errors_reported++ < 6) {
        std::cerr << "LDI " << invalid
                  << ": falscher Ausnahmetyp: " << e.what() << "\n";
      }
    }
  }

  constexpr int kTotal = 8 * 16 + 3;
  std::cout << "Assembler-Encoder: " << passed << "/" << kTotal
            << " Tests bestanden\n";
  return passed == kTotal ? 0 : 1;
}
