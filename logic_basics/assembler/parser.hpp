#pragma once

#include "encoder.hpp"

#include <string>

namespace assembler {

// Eine Quellzeile kann leer sein, ein Label deklarieren oder einen Befehl tragen.
// Die Auswertung von Labeladressen kommt in einer späteren Übung.
enum class LineKind {
  Empty,
  Label,
  Instruction
};

struct ParsedLine {
  LineKind kind = LineKind::Empty;
  std::string label;
  Opcode opcode = Opcode::NOP;
  std::string operand;
};

// Erkennt eine einzelne Assembler-Zeile, ohne Opcode/Operand zu kodieren.
// Leerzeilen und reine Kommentare ergeben LineKind::Empty.
// Syntaxfehler melden sich durch std::invalid_argument.
// Labels werden hier nur gelesen, noch nicht aufgelöst.
ParsedLine parse_line(const std::string& source);

}  // namespace assembler
