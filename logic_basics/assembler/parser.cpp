#include "parser.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace assembler {
namespace {

Opcode find_opcode(const std::string& name) {
  static const std::unordered_map<std::string, Opcode> opcodes = {
      {"ADD", Opcode::ADD}, {"AND", Opcode::AND}, {"OR", Opcode::OR},
      {"XOR", Opcode::XOR}, {"LDI", Opcode::LDI}, {"JMP", Opcode::JMP},
      {"JZ", Opcode::JZ},   {"NOP", Opcode::NOP}};

  const auto it = opcodes.find(name);
  if (it == opcodes.end()) {
    throw std::invalid_argument("Unbekannter Opcode: " + name);
  }
  return it->second;
}

void validate_label_name(const std::string& label) {
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
}

ParsedLine parse_label(const std::string& token, std::istringstream& input) {
  const std::string label = token.substr(0, token.size() - 1);
  validate_label_name(label);

  std::string extra;
  if (input >> extra) {
    throw std::invalid_argument("Unerwartetes weiteres Wort");
  }

  ParsedLine result;
  result.kind = LineKind::Label;
  result.label = label;
  return result;
}

ParsedLine parse_instruction(const std::string& name,
                             std::istringstream& input) {
  const Opcode opcode = find_opcode(name);
  std::string operand;

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

    std::string extra;
    if (input >> extra) {
      throw std::invalid_argument("Zu viele Operanden");
    }
  }

  ParsedLine result;
  result.kind = LineKind::Instruction;
  result.opcode = opcode;
  result.operand = operand;
  return result;
}

}  // namespace

ParsedLine parse_line(const std::string& source) {
  // Nur die Zeichen vor dem ersten Kommentarzeichen betrachten.
  const std::string line = source.substr(0, source.find(';'));
  std::istringstream input(line);
  std::string first;

  if (!(input >> first)) {
    return {};
  }
  if (first.back() == ':') {
    return parse_label(first, input);
  }
  return parse_instruction(first, input);
}

}  // namespace assembler
