#include "pass1.hpp"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using assembler::FirstPassResult;
using assembler::LineKind;
using assembler::Opcode;

using Word = std::pair<Opcode, std::string>;
using Labels = std::unordered_map<std::string, std::size_t>;

struct ValidCase {
  const char* name;
  std::string source;
  std::vector<Word> instructions;
  Labels labels;
};

enum class ErrorKind { InvalidArgument, OutOfRange };

struct ErrorCase {
  const char* name;
  std::string source;
  ErrorKind error;
};

std::string nops(std::size_t count) {
  std::string text;
  for (std::size_t i = 0; i < count; ++i) {
    text += "NOP\n";
  }
  return text;
}

std::vector<Word> expected_nops(std::size_t count) {
  return std::vector<Word>(count, {Opcode::NOP, ""});
}

// Unabhängige Referenzerwartung: Wir rufen encode() oder first_pass()
// zur Berechnung erwarteter Ergebnisse ausdrücklich nicht auf.
bool matches(const FirstPassResult& got, const ValidCase& expected) {
  if (got.instructions.size() != expected.instructions.size() ||
      got.labels != expected.labels) {
    return false;
  }
  for (std::size_t i = 0; i < expected.instructions.size(); ++i) {
    const auto& word = got.instructions[i];
    if (word.kind != LineKind::Instruction ||
        word.opcode != expected.instructions[i].first ||
        word.operand != expected.instructions[i].second ||
        !word.label.empty()) {
      return false;
    }
  }
  return true;
}

}  // namespace

int main() {
  // Jede gültige Anweisung belegt genau eine Adresse; Labelzeilen nicht.
  const std::vector<ValidCase> valid = {
      {"Leeres Programm", "", {}, {}},
      {"Nur Leerraum und Kommentare", "  \n\t; hallo\n; zweite Zeile\r\n", {}, {}},
      {"Ein LDI", "LDI 5", {{Opcode::LDI, "5"}}, {}},
      {"Ein NOP", "NOP\n", {{Opcode::NOP, ""}}, {}},
      {"Kommentar nach Befehl", "ADD 15 ; minus eins\n",
       {{Opcode::ADD, "15"}}, {}},
      {"Zwei Befehle", "LDI 5\nADD 15\n",
       {{Opcode::LDI, "5"}, {Opcode::ADD, "15"}}, {}},
      {"Label am Anfang", "start:\nLDI 5\n",
       {{Opcode::LDI, "5"}}, {{"start", 0}}},
      {"Mehrere Labels vor einem Befehl", "first:\nsecond:\nNOP\n",
       {{Opcode::NOP, ""}}, {{"first", 0}, {"second", 0}}},
      {"Label zwischen Befehlen", "NOP\nmiddle:\nADD 1\n",
       {{Opcode::NOP, ""}, {Opcode::ADD, "1"}}, {{"middle", 1}}},
      {"Label am Ende", "NOP\nend:\n",
       {{Opcode::NOP, ""}}, {{"end", 1}}},
      {"Kommentar und leere Zeilen um Label", " ; Start\n\nstart: ; Label\n\n NOP ; Befehl\n",
       {{Opcode::NOP, ""}}, {{"start", 0}}},
      {"Vorwärts- und Rückwärtsreferenz",
       "entry:\nJMP target\nNOP\ntarget:\nJZ entry\n",
       {{Opcode::JMP, "target"}, {Opcode::NOP, ""}, {Opcode::JZ, "entry"}},
       {{"entry", 0}, {"target", 2}}},
      {"Unbekanntes Sprungziel erst später prüfen", "JMP no_such_label\n",
       {{Opcode::JMP, "no_such_label"}}, {}},
      {"Operand 16 erst später prüfen", "LDI 16\n",
       {{Opcode::LDI, "16"}}, {}},
      {"Hexadezimaltext noch nicht auswerten", "JMP 0xF\n",
       {{Opcode::JMP, "0xF"}}, {}},
      {"Countdown aus CPU v2",
       "; Countdown\nstart:\n LDI 5\nloop:\n ADD 15\n JZ alarm\n JMP loop\n NOP\n"
       "alarm:\n OR 15\n AND 10\n XOR 15\n NOP\n JMP start\n",
       {{Opcode::LDI, "5"}, {Opcode::ADD, "15"}, {Opcode::JZ, "alarm"},
        {Opcode::JMP, "loop"}, {Opcode::NOP, ""}, {Opcode::OR, "15"},
        {Opcode::AND, "10"}, {Opcode::XOR, "15"}, {Opcode::NOP, ""},
        {Opcode::JMP, "start"}},
       {{"start", 0}, {"loop", 1}, {"alarm", 5}}},
      {"Genau 16 Befehle", nops(16), expected_nops(16), {}},
      {"16 Befehle mit trailing Leerraum", nops(16) + "\n; erlaubt\n",
       expected_nops(16), {}},
      {"Labelnamen sind case-sensitive", "a:\nA:\nNOP\n",
       {{Opcode::NOP, ""}}, {{"a", 0}, {"A", 0}}},
      {"Label an Adresse 15", nops(15) + "last:\n",
       expected_nops(15), {{"last", 15}}},
      {"Label vor dem 16. Wort", nops(15) + "last:\nNOP\n",
       expected_nops(16), {{"last", 15}}},
      {"Windows-Zeilenenden", "start:\r\nLDI 5\r\nNOP\r\n",
       {{Opcode::LDI, "5"}, {Opcode::NOP, ""}}, {{"start", 0}}},
      {"Nur ein Label", "begin:\n", {}, {{"begin", 0}}}
  };

  const std::vector<ErrorCase> invalid = {
      {"17 Befehle sind zu viele", nops(17), ErrorKind::OutOfRange},
      {"Label auf Adresse 16 ist unzulässig", nops(16) + "end:\n",
       ErrorKind::OutOfRange},
      {"Doppeltes Label am gleichen PC", "start:\nstart:\nNOP\n",
       ErrorKind::InvalidArgument},
      {"Doppeltes Label an anderer Adresse", "start:\nNOP\nstart:\n",
       ErrorKind::InvalidArgument},
      {"Operand fehlt", "ADD\n", ErrorKind::InvalidArgument},
      {"Unerwartetes Zusatzwort", "JMP loop extra\n",
       ErrorKind::InvalidArgument},
      {"Ungültiger Labelname", "bad-name:\n",
       ErrorKind::InvalidArgument},
      {"Label und Befehl in einer Zeile", "loop: NOP\n",
       ErrorKind::InvalidArgument},
      {"Unbekannter Befehl", "FOO 1\n",
       ErrorKind::InvalidArgument}
  };

  std::size_t passed = 0;
  int shown = 0;
  for (const auto& tc : valid) {
    std::istringstream input(tc.source);
    try {
      const auto result = assembler::first_pass(input);
      if (matches(result, tc)) {
        ++passed;
      } else if (shown++ < 8) {
        std::cerr << "Falsches Ergebnis: " << tc.name
                  << " (Wörter: " << result.instructions.size()
                  << ", Labels: " << result.labels.size() << ")\n";
      }
    } catch (const std::exception& e) {
      if (shown++ < 8) {
        std::cerr << "Unerwartete Exception: " << tc.name
                  << ": " << e.what() << "\n";
      }
    }
  }
  for (const auto& tc : invalid) {
    std::istringstream input(tc.source);
    try {
      (void)assembler::first_pass(input);
      if (shown++ < 8) {
        std::cerr << "Fehler erwartet: " << tc.name << "\n";
      }
    } catch (const std::invalid_argument& e) {
      if (tc.error == ErrorKind::InvalidArgument) {
        ++passed;
      } else if (shown++ < 8) {
        std::cerr << "Falscher Fehlertyp: " << tc.name << ": " << e.what() << "\n";
      }
    } catch (const std::out_of_range& e) {
      if (tc.error == ErrorKind::OutOfRange) {
        ++passed;
      } else if (shown++ < 8) {
        std::cerr << "Falscher Fehlertyp: " << tc.name << ": " << e.what() << "\n";
      }
    }
  }

  const std::size_t total = valid.size() + invalid.size();
  std::cout << "Assembler-Pass1: " << passed << "/" << total
            << " Tests bestanden\n";
  return passed == total ? 0 : 1;
}
