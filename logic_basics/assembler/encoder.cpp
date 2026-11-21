#include "encoder.hpp"

#include <stdexcept>

namespace assembler {

std::uint8_t encode(Opcode opcode, unsigned immediate) {
  // Jeder Operand belegt genau vier Bits. Ungültige Werte nicht abschneiden.
  if (immediate > kMaxImmediate) {
    throw std::out_of_range("Immediate out of range");
  }

  return (static_cast<unsigned>(opcode) << 4) | immediate;
}

}  // namespace assembler
