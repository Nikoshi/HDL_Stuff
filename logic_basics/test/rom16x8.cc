#include "Vrom16x8_top.h"
#include "verilated_vcd_c.h"

#include <array>
#include <iostream>

// Unabhängiges Referenzprogramm aus der CPU-v2-ISA:
// Countdown 5..0 und anschließender LED-Alarm.
// Alle unbenutzten Adressen enthalten ausdrücklich NOP (0xC0).
constexpr std::array<int, 16> kProgram = {
    0x45, 0x0F, 0x95, 0x81,
    0xC0, 0x2F, 0x1A, 0x3F,
    0xC0, 0x80, 0xC0, 0xC0,
    0xC0, 0xC0, 0xC0, 0xC0
};

int main() {
  Verilated::traceEverOn(true);
  Vrom16x8_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/rom16x8.vcd");

  int checks = 0;
  int sim_time = 0;

  auto check = [&](int address, const char* context) -> bool {
    dut.address = address & 0xF;
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    const int expected = kProgram[static_cast<unsigned>(address) & 0xF];
    if (int(dut.instruction) != expected) {
      std::cerr << "CPU-v2-ROM16x8 falsch: " << context
                << " address=" << (address & 0xF)
                << " erwartet=0x" << std::hex << expected
                << " erhalten=0x" << int(dut.instruction)
                << std::dec << "\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checks;
    return true;
  };

  // Jede Adresse aufwärts und abwärts.
  for (int address = 0; address < 16; ++address) {
    if (!check(address, "Aufsteigende Adresse")) return 1;
  }
  for (int address = 15; address >= 0; --address) {
    if (!check(address, "Absteigende Adresse")) return 1;
  }

  // Alle Übergänge zwischen zwei beliebigen Adressen,
  // ohne Takt und ohne dazwischen gespeicherten Zustand.
  for (int from = 0; from < 16; ++from) {
    for (int to = 0; to < 16; ++to) {
      if (!check(from, "Ausgangsadresse") ||
          !check(to, "Direkter Adresswechsel")) return 1;
    }
  }

  // Zusätzliche nicht-monotone Adressfolge.
  for (int i = 0; i < 64; ++i) {
    if (!check((11 * i + 3) & 0xF, "Gemischte Adressfolge")) return 1;
  }

  trace.close();
  dut.final();

  constexpr int expected_checks = 16 + 16 + 16 * 16 * 2 + 64;
  if (checks != expected_checks) {
    std::cerr << "ROM16x8-Testbenchfehler: " << checks
              << "/" << expected_checks << "\n";
    return 1;
  }
  std::cout << "CPU-v2-ROM16x8: " << checks
            << "/" << expected_checks << " Tests bestanden\n";
  return 0;
}
