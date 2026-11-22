# Mini-Assembler, Kapitel III: Erster Programmdurchlauf

**Status:** Offene C++17-Übung. Encoder (131/131) und Zeilenparser
(38/38) wurden lokal vollständig bestanden. Nun verarbeiten wir
ein **ganzes Programm**, ohne bereits Maschinenwörter zu erzeugen.

## 1. Lernziel und Abgrenzung

[Kapitel II](02_parser.md) erkennt den Aufbau genau einer Zeile.
Der erste Programmdurchlauf (**Pass 1**) ruft diesen fertigen Parser
für jede Zeile auf und sammelt zwei Ergebnisse:

1. Die Befehlsfolge in ihrer Reihenfolge.
2. Die Positionen der definierten Labels.

**Noch nicht Teil dieser Übung:** Operanden als Zahlen interpretieren,
JMP-/JZ-Ziele auflösen, den Encoder aufrufen, ROM-Spade-Code ausgeben
oder die CLI in `main.cpp` aktivieren.

## 2. Warum ein ganzer Durchlauf nötig ist

Ein Vorwärtssprung darf eine Marke nennen, die erst später vorkommt:

```asm
start:
    JMP alarm
    NOP
alarm:
    LDI 5
```

Bei `JMP alarm` ist die Zieladresse zunächst unbekannt.
Am Ende des ersten Durchlaufs kann das Ergebnis so aussehen:

| ROM-Adresse | Opcode | Operand als Text |
|---:|---|---|
| 0 | JMP | `"alarm"` |
| 1 | NOP | `""` |
| 2 | LDI | `"5"` |

| Labelname | Adresse |
|---|---:|
| `start` | 0 |
| `alarm` | 2 |

Ein Label bezeichnet immer **die Adresse des nächsten Befehls**.
Die Labelzeile selbst ist **kein** ROM-Wort. Ein zweiter Durchlauf
kann später die Operanden gegen diese Labeltabelle auflösen.

## 3. Die neue Schnittstelle

Dateien: [`pass1.hpp`](../../assembler/pass1.hpp) und
[`pass1.cpp`](../../assembler/pass1.cpp).

```cpp
struct FirstPassResult {
    std::vector<ParsedLine> instructions;
    std::unordered_map<std::string, std::size_t> labels;
};

FirstPassResult first_pass(std::istream& source);
```

`instructions` enthält **nur** `LineKind::Instruction`-Elemente,
in Quellreihenfolge. Ihr jeweiliger Index ist ihre ROM-Adresse.
`labels` verbindet Namen mit Indizes. `Empty`-Zeilen kommen
in keinem der beiden Container vor.

Das Argument ist ein `std::istream&`, damit sich dieselbe Funktion
sowohl aus einem Test mit `std::istringstream` als auch später
aus einem Dateistream aufrufen lässt.

## 4. Zählregel und ROM-Grenze

Der nächste freie ROM-Platz ist die aktuelle Anzahl von Befehlen.
Der Zähler beginnt bei 0. Kommentare, Leerzeilen und Labels erhöhen
ihn nicht. Nur ein Befehl belegt einen Platz.

| Zustand | Vorgabe |
|---|---|
| Kein Befehl | Gültiges leeres Programm |
| 15 Befehle, danach `last:` | Label `last = 15`, gültig |
| 15 Befehle, dann ein weiterer Befehl | 16 belegte Plätze, gültig |
| 16 Befehle, danach nur Kommentare | Gültig |
| 16 Befehle, danach `end:` | **Fehler**: Labeladresse 16 existiert nicht |
| 17. Befehl | **Fehler**: ROM hat nur 16 Einträge |

Die vorhandene Konstante `kRomWords` aus `encoder.hpp`
legt diese Kapazität fest. **Kein Modulo 16 und kein Abschneiden**:
Solche Operationen würden ein zu langes Programm fälschlich gültig machen.

## 5. Fehler und aufgeschobene Prüfungen

| Eingabe | Pass-1-Verhalten |
|---|---|
| Zwei Mal `start:` | `std::invalid_argument` wegen doppeltem Label |
| `FOO 2` oder `ADD` | `std::invalid_argument` aus `parse_line` |
| 17 Befehle | `std::out_of_range` |
| Label auf Adresse 16 | `std::out_of_range` |
| `LDI 16` | Hier noch gültig, Operand `"16"` |
| `JMP missing` | Hier noch gültig, Operand `"missing"` |

Labelnamen sind **case-sensitive**: `loop` und `Loop` sind
zwei verschiedene Namen. Mehrere *verschiedene* Labels dürfen
dieselbe Adresse bezeichnen.

Erst beim nächsten Durchlauf prüfen wir, ob referenzierte Labels
existieren und Immediate-Werte in vier Bits passen.

## 6. Deine Implementierungsaufgabe

Ändere ausschließlich `logic_basics/assembler/pass1.cpp`.
Die Headerdatei und die Tests sind der feste Vertrag.

1. Lese den Eingabestream mit `std::getline` zeilenweise.
2. Verwende die fertige `parse_line`-Funktion.
3. Ignoriere `Empty`.
4. Übernimm `Instruction` in die Befehlsfolge, sofern das ROM Platz hat.
5. Trage `Label` mit der **aktuellen Befehlsanzahl** in die Map ein.
6. Lehne doppelte Labels und Adressen außerhalb des ROM ab.

Das sind die C++17-Bausteine, die du benötigst:
[`std::getline`, `std::vector`, `std::unordered_map` und `std::size_t`](cpp17_collections.md).

Eine hilfreiche Designfrage: Wie kannst du mit `emplace`
ein neues Label eintragen und gleichzeitig erkennen,
ob dieser Name bereits existierte? Die Antwort
steht in der begleitenden C++17-Einführung.

## 7. Testvertrag

Im Repository-Root:

```sh
make test_assembler_pass1
```

Die TDD-Testbench enthält **23 gültige und 9 ungültige Programme**
(32 Fälle). Sie prüft exakte Befehlsreihenfolge, unveränderte
Operandentexte, Labeladressen und die vorgesehenen Exceptiontypen.

Wichtige Grenzfälle: `last:` auf 15, genau 16 Befehle,
das 17. Wort, ein Label auf 16, doppelte Labels, Sprünge auf
noch unbekannte Namen und das 10-Befehls-Countdown-Beispiel.

Der Stub in `pass1.cpp` gibt zunächst ein leeres Ergebnis zurück;
die neue Suite soll daher absichtlich fehlschlagen.
`make test_assembler_encoder` und `make test_assembler_parser`
bleiben die abgeschlossenen Software-Einzeltests.
`make test_completed` bleibt die separate Hardware-Regression.

## 8. Danach

Pass 2 erhält die vollständige Symboltabelle, wertet
Zahlen und Sprungziele aus und kann die vorhandene Funktion
`encode(Opcode, immediate)` verwenden. Der eigentliche
Spade-`match`-Block folgt als eigene Ausgabestufe.

---

[← Zurück: Zeilenparser](02_parser.md) · [Zur Kapitelübersicht](../README.md)
