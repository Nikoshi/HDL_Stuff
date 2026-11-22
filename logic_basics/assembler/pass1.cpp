#include "pass1.hpp"

namespace assembler {

FirstPassResult first_pass(std::istream& source) {
  // TODO: Mit std::getline(...) jede Quelltextzeile lesen.
  // TODO: parse_line(...) aus Kapitel II wiederverwenden.
  // TODO: Nur Anweisungen in instructions speichern.
  // TODO: Labels auf die Adresse der nächsten Anweisung abbilden.
  // TODO: Doppelte Namen und ROM-Adressen außerhalb 0..15 ablehnen.
  // Die eigentliche Logik bleibt als neue Übung offen.
  (void)source;
  return {};
}

}  // namespace assembler
