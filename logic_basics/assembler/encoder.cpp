#include "encoder.hpp"

namespace assembler {

std::uint8_t encode(Opcode opcode, unsigned immediate) {
  // TODO: Wort gemäß CPU-v2-ISA zusammensetzen.
  // TODO: Operanden außerhalb von 0..15 mit std::out_of_range ablehnen.
  // Noch kein funktionierender Encoder: Der erste Test soll fehlschlagen.
  (void)opcode;
  (void)immediate;
  return 0;
}

}  // namespace assembler
