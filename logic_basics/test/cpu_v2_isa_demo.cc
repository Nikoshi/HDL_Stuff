#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

// Eigenstaendiges Softwaremodell der geplanten 8-Bit-ISA.
// Kein Verilator und keine Spade-Hardware: Wir pruefen zuerst das Programm.
namespace {
constexpr std::uint8_t encode(unsigned opcode, unsigned immediate) {
  return static_cast<std::uint8_t>(((opcode & 0xF) << 4) | (immediate & 0xF));
}

// Nur acht definierte Opcodes, andere Werte bleiben reserviert.
enum Opcode : unsigned {
  ADD = 0x0, AND = 0x1, OR = 0x2, XOR = 0x3,
  LDI = 0x4, JMP = 0x8, JZ = 0x9, NOP = 0xC
};

// Festes Testprogramm: Countdown 5..0 und anschliessend LED-Muster.
// Nicht belegte Adressen enthalten NOP, kein zusaetzliches Programm-RAM.
constexpr std::array<std::uint8_t, 16> kProgram = {
    encode(LDI, 5),    // 0: ACC = 5
    encode(ADD, 15),   // 1: ACC = ACC - 1 (modulo 16)
    encode(JZ, 5),     // 2: Bei null den Alarm starten
    encode(JMP, 1),    // 3: Sonst weiter herunterzaehlen
    encode(NOP, 0),    // 4: Nicht erreichbar im Demoablauf
    encode(OR, 15),    // 5: LED-Muster 1111
    encode(AND, 10),   // 6: LED-Muster 1010
    encode(XOR, 15),   // 7: LED-Muster 0101
    encode(NOP, 0),    // 8: Muster fuer einen Takt halten
    encode(JMP, 0),    // 9: Neuen Countdown beginnen
    encode(NOP, 0), encode(NOP, 0), encode(NOP, 0),
    encode(NOP, 0), encode(NOP, 0), encode(NOP, 0)
};

const char* opcode_name(unsigned opcode) {
  switch (opcode) {
    case ADD: return "ADD";
    case AND: return "AND";
    case OR:  return "OR";
    case XOR: return "XOR";
    case LDI: return "LDI";
    case JMP: return "JMP";
    case JZ:  return "JZ";
    case NOP: return "NOP";
    default:  return "RESERVIERT";
  }
}

std::string bits4(unsigned value) {
  std::string result;
  for (int b = 3; b >= 0; --b) result += ((value >> b) & 1) ? '1' : '0';
  return result;
}

struct Snapshot {
  unsigned old_pc;
  unsigned word;
  unsigned old_acc;
  unsigned new_pc;
  unsigned new_acc;
};

struct CpuV2Reference {
  unsigned pc = 0;
  unsigned acc = 0;

  // Ein Aufruf entspricht genau einer steigenden Taktflanke.
  // Alles wird anhand des alten Zustands berechnet.
  Snapshot rising_edge(bool rst = false, bool advance = true) {
    const unsigned old_pc = pc;
    const unsigned old_acc = acc;
    const unsigned word = kProgram.at(old_pc);
    Snapshot result{old_pc, word, old_acc, pc, acc};

    if (rst) {
      pc = 0;
      acc = 0;
    } else if (advance) {
      const unsigned op = word >> 4;
      const unsigned imm = word & 0xF;
      pc = (old_pc + 1) & 0xF;
      switch (op) {
        case ADD: acc = (old_acc + imm) & 0xF; break;
        case AND: acc = old_acc & imm; break;
        case OR:  acc = old_acc | imm; break;
        case XOR: acc = old_acc ^ imm; break;
        case LDI: acc = imm; break;
        case JMP: pc = imm; break;
        case JZ:  if (old_acc == 0) pc = imm; break;
        case NOP: break;
        default: throw std::logic_error("Reservierter Opcode im Programm");
      }
    }

    result.new_pc = pc;
    result.new_acc = acc;
    return result;
  }
};

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

// Die Assertions vergleichen beobachtbare Programmphasen, nicht nur
// Einzelbefehle. Dadurch werden auch Sprungziele und Schleifen geprueft.
void check_program() {
  CpuV2Reference cpu;
  constexpr std::array<unsigned, 10> kExpectedWords = {
      0x45, 0x0F, 0x95, 0x81, 0xC0,
      0x2F, 0x1A, 0x3F, 0xC0, 0x80
  };
  for (unsigned i = 0; i < kExpectedWords.size(); ++i) {
    require(kProgram[i] == kExpectedWords[i], "Falsche Befehlscodierung");
  }

  const auto loaded = cpu.rising_edge();
  require(loaded.old_pc == 0 && loaded.new_pc == 1 && cpu.acc == 5,
          "LDI 5 fehlgeschlagen");

  // Bei 5,4,3,2,1 muss JZ jeweils nicht springen; bei 0 muss es springen.
  for (int expected_acc = 4; expected_acc >= 0; --expected_acc) {
    const auto sub = cpu.rising_edge();
    require(sub.old_pc == 1 && cpu.pc == 2 && cpu.acc == unsigned(expected_acc),
            "Countdown via ADD 15 fehlerhaft");
    const auto branch = cpu.rising_edge();
    require(branch.old_pc == 2 && branch.old_acc == unsigned(expected_acc),
            "JZ muss alten ACC-Wert pruefen");
    if (expected_acc == 0) {
      require(cpu.pc == 5 && cpu.acc == 0, "JZ muss bei Null springen");
    } else {
      require(cpu.pc == 3 && cpu.acc == unsigned(expected_acc),
              "JZ darf bei Nicht-Null nicht springen");
      cpu.rising_edge();
      require(cpu.pc == 1, "Ruecksprung des Countdowns fehlerhaft");
    }
  }

  cpu.rising_edge();
  require(cpu.pc == 6 && cpu.acc == 15, "OR muss alle LEDs einschalten");
  cpu.rising_edge();
  require(cpu.pc == 7 && cpu.acc == 10, "AND muss Muster 1010 erzeugen");
  cpu.rising_edge();
  require(cpu.pc == 8 && cpu.acc == 5, "XOR muss Muster 0101 erzeugen");
  cpu.rising_edge();
  require(cpu.pc == 9 && cpu.acc == 5, "NOP muss ACC halten");
  cpu.rising_edge();
  require(cpu.pc == 0 && cpu.acc == 5, "JMP 0 muss neuen Durchlauf starten");

  // Pausieren und synchroner Reset haben eigene Regeln.
  const auto paused = cpu.rising_edge(false, false);
  require(paused.old_pc == 0 && cpu.pc == 0 && cpu.acc == 5,
          "advance=0 muss PC und ACC halten");
  cpu.rising_edge(true, false);
  require(cpu.pc == 0 && cpu.acc == 0,
          "Synchroner Reset muss auch bei advance=0 wirken");
  for (unsigned i = 0; i < 20; ++i) cpu.rising_edge();
  require(cpu.pc == 0 && cpu.acc == 5,
          "Gesamter 20-Schritt-Programmdurchlauf fehlerhaft");
}

void print_trace() {
  CpuV2Reference cpu;
  std::cout << "CPU v2 ISA-Demo: Countdown und LED-Alarm (Softwaremodell)\n";
  std::cout << "Takt | PC | Wort | Befehl | ACC vorher -> nachher | PC neu | LEDs\n";
  std::cout << "-----+----+------+--------+----------------------+--------+------\n";
  for (int tick = 1; tick <= 21; ++tick) {
    const auto step = cpu.rising_edge();
    const unsigned opcode = step.word >> 4;
    const unsigned imm = step.word & 0xF;
    std::cout << std::setw(4) << tick << " | "
              << std::setw(2) << step.old_pc << " | 0x"
              << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << step.word << std::dec
              << std::setfill(' ') << " | " << std::setw(3)
              << opcode_name(opcode) << " " << std::setw(2) << imm
              << " | " << std::setw(2) << step.old_acc << " -> "
              << std::setw(2) << step.new_acc << "             | "
              << std::setw(2) << step.new_pc << "     | "
              << bits4(step.new_acc) << "\n";
  }
}
}  // namespace

int main() {
  try {
    check_program();
    print_trace();
    std::cout << "\nCPU v2: ISA-Demo und Ablaufpruefungen bestanden.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "CPU v2 ISA-Demo fehlgeschlagen: " << error.what() << '\n';
    return 1;
  }
}
