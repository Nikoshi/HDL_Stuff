#include "encoder.hpp"

#include <stdexcept>

namespace assembler {

std::uint8_t encode(Opcode opcode, unsigned immediate) {
  // TODO: Wort gemäß CPU-v2-ISA zusammensetzen.
  // TODO: Operanden außerhalb von 0..15 mit std::out_of_range ablehnen.
  // Noch kein funktionierender Encoder: Der erste Test soll fehlschlagen.
  if (immediate > 15)
    throw std::out_of_range("Immediate out of range");

  return (static_cast<unsigned>(opcode) << 4) | immediate;
}

} // namespace assembler
