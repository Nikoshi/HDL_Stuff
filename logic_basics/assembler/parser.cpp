#include "parser.hpp"
#include "encoder.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace assembler {

Opcode find_opcode(const std::string &op) {
  static const std::unordered_map<std::string, Opcode> opcodes = {
      {"ADD", Opcode::ADD}, {"AND", Opcode::AND}, {"OR", Opcode::OR},
      {"XOR", Opcode::XOR}, {"LDI", Opcode::LDI}, {"JMP", Opcode::JMP},
      {"JZ", Opcode::JZ},   {"NOP", Opcode::NOP}};

  const auto it = opcodes.find(op);

  if (it == opcodes.end()) {
    throw std::invalid_argument("Unbekannter Opcode: " + op);
  }

  return it->second;
}

ParsedLine parse_line(const std::string &source) {
  // TODO: Kommentar ab ';' ignorieren.
  auto comment_position = source.find(';');

  std::string line = source;
  if (comment_position != std::string::npos) {
    line = source.substr(0, comment_position);
  }

  // TODO: Leerzeile, Label oder Instruktion erkennen.

  std::istringstream input(line);
  std::string op;
  std::string operand;
  std::string extra;

  // Leerzeilen erkennen
  if (!(input >> op)) {
    return {};
  }

  // Label erkennen
  if (!op.empty() && op.back() == ':') {
    std::string label = op.substr(0, op.size() - 1);

    if (label.empty() ||
        (!std::isalpha(static_cast<unsigned char>(label.front())) &&
         label.front() != '_')) {
      throw std::invalid_argument("Ungültiger Labelanfang");
    }

    for (char c : label) {
      if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
        throw std::invalid_argument("Ungültiges Label");
      }
    }

    if (input >> operand) {
      throw std::invalid_argument("Unerwartetes weiteres Wort");
    }

    auto parsedLine = ParsedLine();
    parsedLine.kind = LineKind::Label;
    parsedLine.label = label;

    return parsedLine;
  }

  auto opcode = find_opcode(op);
  auto parsedLine = ParsedLine();

  if (opcode == Opcode::NOP) {
    if (input >> operand) {
      throw std::invalid_argument("NOP erwartet keinen Operanden");
    }
  } else {
    if (!(input >> operand)) {
      throw std::invalid_argument("Operand fehlt");
    }

    if (operand.find(':') != std::string::npos) {
      throw std::invalid_argument("Unerwarteter Doppelpunkt im Operanden");
    }

    if (input >> extra) {
      throw std::invalid_argument("Zu viele Operanden");
    }
  }

  parsedLine.kind = LineKind::Instruction;
  parsedLine.opcode = opcode;
  parsedLine.operand = operand;

  return parsedLine;
}

} // namespace assembler
