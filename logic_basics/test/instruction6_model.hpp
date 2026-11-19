#pragma once

// Software-Referenzmodell der bisher definierten CPU-Befehle.
// Es verwendet keine Spade-Schaltung und keinen Verilator-Zustand.
struct Instruction6Model {
  int acc = 0;
  bool initialized = false;

  void rising_edge(bool rst, bool enable, int instruction) {
    if (rst) {
      acc = 0;
      initialized = true;
      return;
    }
    if (!initialized || !enable) return;

    const int opcode = (instruction >> 4) & 0x3;
    const int immediate = instruction & 0xF;

    switch (opcode) {
      case 0: acc = acc + immediate; break;
      case 1: acc = acc & immediate; break;
      case 2: acc = acc | immediate; break;
      case 3: acc = acc ^ immediate; break;
    }
    acc &= 0xF;
  }
};
