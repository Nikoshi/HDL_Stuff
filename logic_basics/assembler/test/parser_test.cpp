#include "parser.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using assembler::LineKind;
using assembler::Opcode;
using assembler::ParsedLine;

struct ValidCase {
  const char* source;
  LineKind kind;
  Opcode opcode;
  const char* operand;
  const char* label;
};

constexpr ValidCase kValid[] = {
    {"",                   LineKind::Empty,       Opcode::NOP, "",         ""},
    {"    ",               LineKind::Empty,       Opcode::NOP, "",         ""},
    {" \t \r",             LineKind::Empty,       Opcode::NOP, "",         ""},
    {";",                  LineKind::Empty,       Opcode::NOP, "",         ""},
    {"  ; Kommentar",      LineKind::Empty,       Opcode::NOP, "",         ""},

    {"LDI 5",              LineKind::Instruction, Opcode::LDI, "5",        ""},
    {"  LDI   5  ",        LineKind::Instruction, Opcode::LDI, "5",        ""},
    {"ADD 15 ; minus 1",   LineKind::Instruction, Opcode::ADD, "15",       ""},
    {"AND\t10",            LineKind::Instruction, Opcode::AND, "10",       ""},
    {"OR 15",              LineKind::Instruction, Opcode::OR,  "15",       ""},
    {"XOR 0",              LineKind::Instruction, Opcode::XOR, "0",        ""},
    {"JMP loop",           LineKind::Instruction, Opcode::JMP, "loop",     ""},
    {"JZ alarm",           LineKind::Instruction, Opcode::JZ,  "alarm",    ""},
    {"NOP",                LineKind::Instruction, Opcode::NOP, "",         ""},
    {" NOP ; halt",        LineKind::Instruction, Opcode::NOP, "",         ""},
    {"LDI 16",             LineKind::Instruction, Opcode::LDI, "16",       ""},
    {"JMP 0xF",            LineKind::Instruction, Opcode::JMP, "0xF",      ""},
    {"ADD 0\r",            LineKind::Instruction, Opcode::ADD, "0",        ""},
    {"\tXOR\t7;Kommentar", LineKind::Instruction, Opcode::XOR, "7",        ""},
    {"JZ _finish2",        LineKind::Instruction, Opcode::JZ,  "_finish2", ""},
    {"start:",             LineKind::Label,       Opcode::NOP, "",         "start"},
    {" loop: ; Ziel",      LineKind::Label,       Opcode::NOP, "",         "loop"},
    {"_entry9:",           LineKind::Label,       Opcode::NOP, "",         "_entry9"},
    {"alarm:\r",           LineKind::Label,       Opcode::NOP, "",         "alarm"},
};

constexpr const char* kInvalid[] = {
    "FOO 2",            // Unbekannter Befehl
    "ldi 1",            // Mnemonics sind großgeschrieben
    "ADD",              // Operand fehlt
    "NOP 0",            // NOP akzeptiert keinen Textoperanden
    "ADD 1 2",          // Zu viele Operanden
    "JMP loop extra",   // Zu viele Token
    "loop: NOP",        // Label und Befehl dürfen noch nicht in einer Zeile stehen
    "1bad:",            // Label beginnt nicht mit Buchstabe/Unterstrich
    "bad-name:",        // Ungültiges Labelzeichen
    ":",                // Leerer Labelname
    "ADD ; fehlt",      // Kommentar ersetzt keinen Operanden
    "JMP label:",       // Operand ist kein Labeldeklarator
    "loop :",           // Leerzeichen vor dem Doppelpunkt
    "NOP foo",          // NOP benötigt keinen Operanden
};

bool matches(const ParsedLine& got, const ValidCase& expected) {
  if (got.kind != expected.kind) {
    return false;
  }
  switch (expected.kind) {
    case LineKind::Empty:
      return got.label.empty() && got.operand.empty();
    case LineKind::Label:
      return got.label == expected.label && got.operand.empty();
    case LineKind::Instruction:
      return got.opcode == expected.opcode && got.operand == expected.operand
          && got.label.empty();
  }
  return false;
}

}  // namespace

int main() {
  int passed = 0;
  int printed = 0;
  for (const auto& tc : kValid) {
    try {
      if (matches(assembler::parse_line(tc.source), tc)) {
        ++passed;
      } else if (printed++ < 8) {
        std::cerr << "Falsch geparst: [" << tc.source << "]\n";
      }
    } catch (const std::exception& e) {
      if (printed++ < 8) {
        std::cerr << "Unerwartete Exception: [" << tc.source << "]: "
                  << e.what() << "\n";
      }
    }
  }

  for (const auto* input : kInvalid) {
    try {
      (void)assembler::parse_line(input);
      if (printed++ < 8) {
        std::cerr << "std::invalid_argument erwartet: [" << input << "]\n";
      }
    } catch (const std::invalid_argument&) {
      ++passed;
    } catch (const std::exception& e) {
      if (printed++ < 8) {
        std::cerr << "Falscher Exceptiontyp: [" << input << "]: "
                  << e.what() << "\n";
      }
    }
  }

  constexpr int kTotal = static_cast<int>(
      sizeof(kValid) / sizeof(kValid[0])
    + sizeof(kInvalid) / sizeof(kInvalid[0]));
  std::cout << "Assembler-Parser: " << passed << "/" << kTotal
            << " Tests bestanden\n";
  return passed == kTotal ? 0 : 1;
}
