# Mini-Assembler, Kapitel II: Zeilen und Befehle parsen

**Status:** Abgeschlossen. Der Zeilenparser wurde mit **38/38 Tests** bestanden gemeldet und anschließend ohne Änderung seines Verhaltens in Hilfsfunktionen gegliedert.
Der Maschinenwort-Encoder aus [Kapitel I](01_encoder.md) bestand **131/131 Tests**.

## 1. Was wir jetzt bauen

Ein Assembler muss unterscheiden, ob eine Quelltextzeile eine
Anweisung, eine Markierung oder nur eine Bemerkung enthält.
Beispiele aus [countdown.asm](../../assembler/examples/countdown.asm):

```asm
start:             ; Marke
    LDI 5          ; Befehl mit Zahl
loop:
    ADD 15         ; Operand als Text
    JZ alarm       ; Sprungziel als Name
    NOP            ; kein Operand
```

Der Parser **kodiert noch kein Byte** und berechnet **keine Adressen**.
Seine einzige Aufgabe ist, den Aufbau einer Zeile zu erkennen und
die Bestandteile korrekt aufzubewahren.

## 2. Drei mögliche Ergebnisse

Die Schnittstelle steht in
[`parser.hpp`](../../assembler/parser.hpp):

```cpp
enum class LineKind { Empty, Label, Instruction };

struct ParsedLine {
  LineKind kind = LineKind::Empty;
  std::string label;
  Opcode opcode = Opcode::NOP;
  std::string operand;
};

ParsedLine parse_line(const std::string& source);
```

| Eingabe | `kind` | Relevante Felder |
|---|---|---|
| `"    "` | `Empty` | keine |
| `"; Kommentar"` | `Empty` | keine |
| `"loop:"` | `Label` | `label = "loop"` |
| `"LDI 5"` | `Instruction` | `opcode = LDI`, `operand = "5"` |
| `"JMP loop"` | `Instruction` | `opcode = JMP`, `operand = "loop"` |
| `"NOP"` | `Instruction` | `opcode = NOP`, `operand = ""` |

Die ungenutzten Felder bleiben auf ihren Standardwerten.
Ein isoliertes Label belegt **kein** ROM-Wort. Nur ein später
erfolgreich kodierter Befehl zählt als Anweisung.

## 3. Syntaxvertrag

- Leerzeichen, Tabs und Windows-Zeilenenden (`\r`) dürfen vorkommen.
- Ab dem ersten Semikolon (`;`) ist der Rest der Zeile Kommentar.
- Alle acht Mnemonics der CPU v2 werden **großgeschrieben**:
  `ADD`, `AND`, `OR`, `XOR`, `LDI`, `JMP`, `JZ`, `NOP`.
- `NOP` akzeptiert **keinen** Operandentext.
- Jeder andere Befehl benötigt **genau ein** Argument, ohne
  weitere Wörter.
- Das Argument bleibt vorerst `std::string`:
  `"15"`, `"0xF"` und `"alarm"` sind zunächst gleichermaßen
  gültige Zeichenfolgen.
- Ein Label steht allein in einer Zeile und endet mit `:`.
  Sein Name beginnt mit einem Buchstaben oder Unterstrich,
  gefolgt von Buchstaben, Ziffern oder Unterstrichen. Zwischen
  Name und Doppelpunkt darf kein Leerraum stehen (also `loop:`,
  nicht `loop :`).
- `loop: NOP` unterstützen wir in dieser Version noch nicht.
  Die vorhandene Beispieldatei verwendet separate Labelzeilen.
  Doppelpunkte gehören ausschließlich zu Label-Deklarationen,
  nicht zu Operanden: `JMP label:` ist daher ungültig.
- Bei unbekannten Befehlen oder ungültiger Syntax
  wirft `parse_line` ein `std::invalid_argument`.

Die Schreibweise ist absichtlich streng. So können
Schreibfehler nicht stillschweigend zu einem anderen
Maschinenbefehl werden.

### Die wichtigste Trennung

`LDI 16` ist **syntaktisch gültig** und muss vom Parser
als `operand = "16"` zurückgegeben werden. Die spätere
Operandenauswertung weist den Wert zurück, weil er nicht
in vier Bits passt.

Ebenso ist `JMP alarm` eine gültige geparste Anweisung.
Ob `alarm` existiert und welche Adresse es trägt, ist
noch unbekannt. Diese Prüfung kommt zusammen mit der
Labeltabelle.

Damit lassen wir Parse-, Adress- und Kodierfehler nicht
durcheinanderlaufen.

## 4. C++17-Werkzeuge für diese Aufgabe

Als Ergänzung zur
[ersten C++17-Einführung](cpp17_einstieg.md) kommen hinzu:

### `std::string`

Strings speichern Text, lassen sich vergleichen und aufteilen.
Für ein Beispiel, das nicht unseren Assembler implementiert:

```cpp
std::string nachricht = "rot ; Signal";
auto pos = nachricht.find(';');

if (pos != std::string::npos) {
    nachricht = nachricht.substr(0, pos);
}
// nachricht ist jetzt "rot "
```

### `std::istringstream`

Ein Eingabestringstream kann an Leerraum getrennte Wörter
lesen. Das ist nützlich, weil wir unterschiedlich viele
Leerzeichen und Tabs zulassen:

```cpp
#include <sstream>
#include <string>

std::istringstream input("Temperatur   21");
std::string wort;
unsigned wert = 0;
input >> wort >> wert;
```

Bei unserer späteren Parserarbeit ist **der Operand zunächst
Text**. Zahlenkonvertierung gehört noch nicht dazu.
Auch das Prüfen auf ein unerwartetes drittes Wort ist wichtig.

### `std::invalid_argument`

Diese Ausnahme aus `<stdexcept>` passt zu fehlerhaften
Eingabezeilen. Der bestehende Encoder verwendet hingegen
`std::out_of_range` für einen bereits erkannten Zahlenwert,
der nicht in das Zielfeld passt.

## 5. Deine Implementierungsaufgabe

Ändere **nur** [`parser.cpp`](../../assembler/parser.cpp).
Die Funktion wurde ursprünglich als unvollständiger Platzhalter bereitgestellt.
Die Implementierung ist abgeschlossen und bleibt als Lernreferenz erhalten.
Der ursprüngliche Übungsplan:

1. Entferne den Kommentarbereich ab `;`.
2. Erkenne leere Zeilen ohne Syntaxfehler.
3. Erkenne Labelzeilen und prüfe den Namen.
4. Ordne Befehlsnamen dem vorhandenen `Opcode`-Enum zu.
5. Prüfe, ob Anzahl und Form der Operanden stimmen.
6. Gib für falsche Eingaben `std::invalid_argument` aus.

Zwei sinnvolle Hilfsfunktionen können die Logik klarer
machen: eine für die Opcode-Erkennung, eine für gültige
Labelnamen. **Das ist ein Entwurfsvorschlag, kein Muss.**

Der Encoder in `encoder.cpp` wird nicht verändert und
aus `parse_line` auch noch nicht aufgerufen.

## 6. Testvertrag

Im Repository-Root:

```sh
make test_assembler_parser
```

Die unabhängige Testbench enthält **24 gültige Zeilen**
und **14 gezielt fehlerhafte Zeilen**, insgesamt **38 Tests**.
Dazu gehören Kommentare, Leerraum, alle Mnemonics,
symbolische Operanden, Labels und falsche Tokenanzahlen.

Der vollständig implementierte Parser wurde lokal mit **38/38 Tests**
bestanden gemeldet. Der Refactoring-Schritt teilt die Logik in
`find_opcode()`, `validate_label_name()`, `parse_label()` und
`parse_instruction()` auf; `parse_line()` übernimmt die Zeilenvorbereitung
und verzweigt an die passende Hilfsfunktion.

`make test_assembler_encoder` bleibt als abgeschlossener
Software-Einzeltest verfügbar. Die Hardware-Regression
bleibt unverändert.

## 7. Was erst im nächsten Schritt passiert

Wenn der Parser funktioniert, lesen wir mit
`std::getline` eine komplette Datei Zeile für Zeile,
zählen die **Befehle** und merken uns die Positionen
von Labels. Erst dann begrenzen wir das ganze Programm
auf die 16 ROM-Adressen und lösen Sprungziele auf.

---

[← Zurück: Encoder](01_encoder.md) · [Zur Kapitelübersicht](../README.md)
