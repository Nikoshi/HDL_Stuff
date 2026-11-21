# C++17-Kurzeinführung für den Mini-Assembler

Diese Seite erklärt die C++-Sprachmittel, die wir im ersten
Assembler-Schritt benötigen. Die eigentliche Aufgabenstellung
steht in [Kapitel I: Encoder](01_encoder.md).

## 1. Wie die Dateien zusammenarbeiten

```text
assembler/
├── encoder.hpp             Öffentliche Typen und Funktionsdeklaration
├── encoder.cpp             Deine Implementierung
├── main.cpp                CLI-Platzhalter für später
├── test/encoder_test.cpp   Unabhängige Encoder-Tests
└── examples/countdown.asm  Spätere Assembler-Eingabe
```

Eine Headerdatei (`.hpp`) legt fest, **welche** Funktionen und
Typen andere Dateien verwenden können. Die `.cpp`-Datei definiert,
**was** eine Funktion ausführt. Der Compiler übersetzt die Dateien
getrennt und verknüpft sie zu einem Programm.

```cpp
// rechner.hpp
#pragma once
unsigned verdoppeln(unsigned wert);

// rechner.cpp
#include "rechner.hpp"
unsigned verdoppeln(unsigned wert) {
    return wert * 2;
}
```

`#pragma once` verhindert, dass ein Header in derselben
Übersetzungseinheit versehentlich mehrfach eingebunden wird.

## 2. Typen: `unsigned` und `std::uint8_t`

`unsigned` ist ein nichtnegativer Ganzzahltyp mit mindestens
16 Bit. In unserem Funktionsargument ist er bewusst breiter
als ein 4-Bit-Feld, damit man **ungültige Werte erkennen**
kann. Würden wir den Operand vor der Prüfung auf vier Bits
reduzieren, wäre der ursprüngliche Fehler nicht mehr sichtbar.

`std::uint8_t` aus `<cstdint>` ist ein Ganzzahltyp
mit exakt acht Bits. Er ist passend für ein CPU-v2-Maschinenwort.
`std::uint8_t` wird bei Ausgaben häufig wie ein Zeichen
behandelt; für eine Zahl kann man ihn zu `unsigned` umwandeln.

```cpp
#include <cstdint>
#include <iostream>

std::uint8_t wert = 0x41;
std::cout << unsigned(wert) << "\n"; // 65
```

## 3. `enum class` und `namespace`

Ein `enum class` gibt erlaubten Varianten Namen.
Anders als ein einfaches Integer kann er nicht stillschweigend
mit anderen Ganzzahlen vermischt werden.

```cpp
enum class Richtung { Links, Rechts };
Richtung r = Richtung::Links;
```

Unsere Headerdatei definiert `assembler::Opcode` mit den
Befehlsnamen der ISA und den zugehörigen Zahlenwerten.
Der Namespace `assembler` trennt diese Namen von anderen
Projektdateien. Innerhalb von `namespace assembler { ... }`
schreibst du `Opcode`, außerhalb `assembler::Opcode`.

Falls du eine Zahl aus einem `enum class` brauchst,
verwendest du eine **explizite Umwandlung**:

```cpp
unsigned zahl = static_cast<unsigned>(Richtung::Rechts);
```

Das Beispiel dient nur der Syntax; die konkreten Opcode-Werte
stehen in `encoder.hpp`.

## 4. Bitoperatoren

| Operator | Wirkung | Beispiel |
|---|---|---|
| `<<` | Bits nach links verschieben | `1u << 3` ergibt 8 |
| `>>` | Bits nach rechts verschieben | `8u >> 2` ergibt 2 |
| `|` | Bits bitweise ODER-verknüpfen | `0b1000u \| 0b0010u` ergibt 10 |
| `&` | Bits bitweise UND-verknüpfen | `0b1100u & 0b0100u` ergibt 4 |

Diese Operatoren arbeiten auf Bitmustern, nicht auf Text.

**Nicht verwechseln:** `|` ist bitweises ODER,
`||` ist logisches ODER für Wahrheitswerte.

## 5. Fehler sichtbar machen

In C++ kannst du eine Funktion mit `throw` abbrechen:

```cpp
#include <stdexcept>

void pruefe_alter(unsigned alter) {
    if (alter > 150) {
        throw std::out_of_range("Alter ausserhalb des Beispiels");
    }
}
```

Ein Test kann die Ausnahme mit `try` und `catch`
gezielt erwarten:

```cpp
try {
    pruefe_alter(200);
} catch (const std::out_of_range&) {
    // Der beabsichtigte Fehlerfall wurde erkannt.
}
```

Für den Encoder ist die **Reihenfolge** wichtig:
erst prüfen, dann Bits zusammensetzen.

## 6. Tests, Schleifen und der Build

Die Testbench verwendet `std::array` (festes Array),
`for`-Schleifen, `std::cout`/`std::cerr` und einen
Rückgabewert aus `main()`:

- `return 0;`: erfolgreich.
- `return 1;`: Test oder Programm fehlgeschlagen.
- `std::cout`: normale Ausgabe.
- `std::cerr`: Fehlermeldung.

Das Makefile ruft den C++-Compiler mit
`-std=c++17 -Wall -Wextra -pedantic` auf.
`-Ilogic_basics/assembler` gibt das Verzeichnis an,
in dem `#include "encoder.hpp"` gesucht wird.
Der `test_assembler_encoder` benötigt weder Verilator
noch Spade und verändert kein CPU-Modul.

## 7. Später im Assembler

Erst beim Einlesen der Assembler-Quelldateien verwenden
wir `std::string` für Text, `std::vector` für die Liste
der Befehle und eine Zuordnung von Labels zu Adressen.
Der Encoder benötigt diese Datenstrukturen noch nicht.

---

[Zur ersten Übung: Maschinenwort-Encoder](01_encoder.md) ·
[Weiter: Textverarbeitung](cpp17_text.md)
