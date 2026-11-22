#pragma once

#include "parser.hpp"

#include <cstddef>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace assembler {

// Ergebnis des ersten Durchgangs, noch ohne Maschinenwort-Kodierung.
// Der Index in instructions ist die spätere ROM-Adresse eines Befehls.
struct FirstPassResult {
  std::vector<ParsedLine> instructions;
  std::unordered_map<std::string, std::size_t> labels;
};

// Liest ein vollständiges Programm. Labels belegen keine ROM-Adresse.
// Fehler: doppelte Labels -> std::invalid_argument;
//         mehr als 16 Befehle oder Labeladresse >= 16 -> std::out_of_range.
// Syntaxfehler aus parse_line werden unverändert weitergegeben.
FirstPassResult first_pass(std::istream& source);

}  // namespace assembler
