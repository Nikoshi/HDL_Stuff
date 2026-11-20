#include "Vrom16x6_top.h"
#include "verilated_vcd_c.h"

#include <array>
#include <iostream>

// Unabhaengiges Referenzprogramm: Die festgelegten 16 Maschinenbefehle.
// Nur der Test benutzt diese Daten; er liest keine Spade-Implementierung.
constexpr std::array<int, 16> kProgram = {
    0x25, 0x0B, 0x37, 0x13,
    0x01, 0x3F, 0x28, 0x16,
    0x0E, 0x32, 0x24, 0x03,
    0x19, 0x3B, 0x22, 0x05
};

int main() {
  Verilated::traceEverOn(true);
  Vrom16x6_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/rom16x6.vcd");

  int checked = 0;
  int sim_time = 0;

  auto check = [&](int address, const char *context) -> bool {
    dut.address = address & 0xF;
    dut.eval();
    trace.dump(sim_time);
    sim_time += 10;

    const int expected = kProgram[static_cast<unsigned>(address & 0xF)];
    if (int(dut.instruction) != expected) {
      std::cerr << "ROM16x6 falsch: " << context
                << " (address=" << (address & 0xF)
                << "): erwartet instruction=" << expected
                << ", erhalten=" << int(dut.instruction) << "\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checked;
    return true;
  };

  // Alle Adressen in beiden Richtungen durchlaufen.
  for (int address = 0; address < 16; ++address) {
    if (!check(address, "Aufsteigende Adresse")) return 1;
  }
  for (int address = 15; address >= 0; --address) {
    if (!check(address, "Absteigende Adresse")) return 1;
  }

  // Aus jeder Startadresse jede Zieladresse ohne Taktflanke anlegen.
  // Der kombinatorische ROM muss sofort den neuen Befehl ausgeben.
  for (int from = 0; from < 16; ++from) {
    for (int to = 0; to < 16; ++to) {
      if (!check(from, "Ausgangsadresse") ||
          !check(to, "Direkter Adresswechsel")) {
        return 1;
      }
    }
  }

  // Weitere Folge: Wiederholungen und groessere Adressspruenge.
  for (int i = 0; i < 64; ++i) {
    if (!check((11 * i + 3) & 0xF, "Gemischte Adressfolge")) return 1;
  }

  trace.close();
  dut.final();

  constexpr int expected_checks = 16 + 16 + (16 * 16 * 2) + 64;
  if (checked != expected_checks) {
    std::cerr << "ROM16x6-Testbench-Fehler: " << checked << "/"
              << expected_checks << " Pruefungen\n";
    return 1;
  }

  std::cout << "16x6-Bit-ROM: " << checked << "/" << expected_checks
            << " Tests bestanden\n";
  return 0;
}
