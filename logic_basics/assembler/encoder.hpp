#pragma once

#include <cstddef>
#include <cstdint>

namespace assembler {

// CPU v2: 4-Bit-PC und festes ROM mit 16 Einträgen zu je 8 Bit.
constexpr std::size_t kRomWords = 16;
constexpr unsigned kMaxImmediate = 15;
constexpr std::uint8_t kNopWord = 0xC0;

// Die Werte entsprechen direkt den oberen vier Bits der CPU-v2-ISA.
// Reservierte Opcodes sind absichtlich nicht aufgeführt.
enum class Opcode : std::uint8_t {
  ADD = 0x0,
  AND = 0x1,
  OR  = 0x2,
  XOR = 0x3,
  LDI = 0x4,
  JMP = 0x8,
  JZ  = 0x9,
  NOP = 0xC
};

// Erste Übung: Opcode (4 Bit) und Immediate (4 Bit) kodieren.
// Bei immediate > 15 eine std::out_of_range auslösen.
std::uint8_t encode(Opcode opcode, unsigned immediate);

}  // namespace assembler
