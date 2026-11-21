#include <iostream>

// Das Kommandozeilenprogramm kommt in einer späteren Etappe.
// Aktuell implementieren wir zuerst den separat testbaren Encoder.
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Aufruf: mini_assembler <programm.asm>\n";
    return 2;
  }

  std::cerr << "Assembler noch nicht implementiert: " << argv[1] << "\n";
  return 1;
}
