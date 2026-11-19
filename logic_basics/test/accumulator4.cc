#include "Vaccumulator4_top.h"
#include "verilated_vcd_c.h"

#include <iostream>

namespace {
int alu_result(int acc, int b, int op) {
  // Unabhängiges Referenzmodell auf vier Bit begrenzen.
  switch (op) {
    case 0: return (acc + b) & 0xF;
    case 1: return acc & b;
    case 2: return acc | b;
    case 3: return acc ^ b;
    default: return 0;
  }
}
}

int main() {
  Verilated::traceEverOn(true);
  Vaccumulator4_top dut;
  VerilatedVcdC trace;
  dut.trace(&trace, 99);
  trace.open("build/accumulator4.vcd");

  int acc = 0;
  int last_clk = 0;
  int sim_time = 0;
  int checked = 0;
  bool initialized = false;

  auto step = [&](int clk, int rst, int enable, int b, int op,
                  const char *description) -> bool {
    dut.clk = clk;
    dut.rst = rst;
    dut.enable = enable;
    dut.b = b & 0xF;
    dut.op = op & 0x3;
    dut.eval();

    // Der einzige Zeitpunkt, an dem sich der gespeicherte Wert ändern darf.
    if (last_clk == 0 && clk == 1) {
      if (rst) {
        acc = 0;
        initialized = true;
      } else if (enable && initialized) {
        acc = alu_result(acc, b & 0xF, op & 0x3);
      }
    }
    last_clk = clk;

    trace.dump(sim_time);
    sim_time += 10;

    if (!initialized) {
      return true;  // Vor dem ersten Reset ist der Registerwert undefiniert.
    }

    if (int(dut.q) != acc) {
      std::cerr << "4-Bit-Akkumulator falsch: " << description
                << " (clk=" << clk << ", rst=" << rst
                << ", enable=" << enable << ", b=" << b
                << ", op=" << op << "): erwartet ACC=" << acc
                << ", erhalten q=" << int(dut.q) << "\n";
      trace.close();
      dut.final();
      return false;
    }
    ++checked;
    return true;
  };

  // Ein kompletter Takt mit zusätzlichen Eingangsänderungen.
  // Eingänge bei hohem Takt dürfen das bereits gespeicherte ACC nicht ändern.
  auto tick = [&](int rst, int enable, int b, int op,
                  const char *description) -> bool {
    return step(0, rst, enable, b, op, "Eingänge vor steigender Flanke") &&
           step(1, rst, enable, b, op, description) &&
           step(1, 0, 1 - enable, (b + 5) & 0xF, (op + 1) & 3,
                "Eingangsänderung bei hohem Takt") &&
           step(0, 0, 1 - enable, (b + 5) & 0xF, (op + 1) & 3,
                "Fallende Taktflanke");
  };

  // Ohne Anfangszustand ist kein q-Wert vorgeschrieben.
  if (!step(0, 1, 1, 15, 0, "Synchronen Reset vorbereiten") ||
      !step(1, 1, 1, 15, 0, "Reset hat Vorrang vor ALU-Enable") ||
      !step(1, 0, 1, 5, 2, "Reset-Freigabe bei hohem Takt") ||
      !step(0, 0, 1, 5, 2, "Fallende Flanke nach Reset")) {
    return 1;
  }

  // Alle 4 Operationen für jeden möglichen gespeicherten ACC-Wert
  // und jeden möglichen Operanden: 4 * 16 * 16 = 1024 ALU-Updates.
  // Für jeden Fall ACC deterministisch herstellen: Reset auf 0, dann
  // OR mit dem gewünschten Wert (0 OR a = a).
  for (int op = 0; op < 4; ++op) {
    for (int a = 0; a < 16; ++a) {
      for (int b = 0; b < 16; ++b) {
        if (!tick(1, 1, 15, 0, "Reset vor jeder Kombination") ||
            !tick(0, 1, a, 2, "Startwert durch OR laden") ||
            !tick(0, 1, b, op, "ALU-Ergebnis in ACC übernehmen") ||
            !tick(0, 0, (~b) & 0xF, op, "Ohne Enable gespeichert halten")) {
          return 1;
        }
      }
    }
  }

  // Mehrere Berechnungen ohne Rücksetzen hintereinander ausführen.
  for (int i = 0; i < 96; ++i) {
    const int b = (11 * i + 7) & 0xF;
    const int op = (3 * i + 1) & 3;
    if (!tick(0, 1, b, op, "Akkumulierte Rechenfolge")) {
      return 1;
    }
  }

  // Reset auch bei ausgeschaltetem Enable muss auf der nächsten
  // steigenden Flanke wirken, nicht schon beim Setzen von rst.
  if (!step(0, 1, 0, 9, 0, "Reset ohne Takt darf ACC nicht ändern") ||
      !step(1, 1, 0, 9, 0, "Synchroner Reset trotz enable=0") ||
      !step(0, 0, 0, 9, 0, "Nach Reset weiter halten")) {
    return 1;
  }

  trace.close();
  dut.final();
  std::cout << "4-Bit-Akkumulator: " << checked
            << "/" << checked << " Tests bestanden\n";
  return 0;
}
