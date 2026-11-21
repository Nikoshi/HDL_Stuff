#include "parser.hpp"

#include <stdexcept>
#include <sstream>

namespace assembler {

ParsedLine parse_line(const std::string& source) {
  // TODO: Kommentar ab ';' ignorieren.
  // TODO: Leerzeile, Label oder Instruktion erkennen.
  // TODO: Den Operandentext unverändert übernehmen.
  // TODO: Ungültige Syntax mit std::invalid_argument ablehnen.
  // Hier noch keine Labeladresse berechnen oder den Encoder aufrufen.
  (void)source;
  return {};
}

}  // namespace assembler
