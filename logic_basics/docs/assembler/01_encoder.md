# Mini-Assembler, Kapitel I: Maschinenwörter kodieren

**Status:** Offene C++17-Übung. Die CPU-v2-Hardware ist abgeschlossen;
der Assembler wird als eigenständiges Softwareprojekt entwickelt.

## 1. Ziel der ersten Etappe

Die CPU v2 versteht nur 8-Bit-Maschinenwörter. Menschen schreiben
lieber `LDI 5` als `0x45`. Der Assembler soll später zwischen diesen
Darstellungen übersetzen und den `match address`-Block eines Spade-ROM
ausgeben.

**In diesem Kapitel implementierst du ausschließlich den Encoder**:
Aus einem bereits bekannten Opcode und einem numerischen Immediate
entsteht ein 8-Bit-Wort. Quelltext lesen, Labels auflösen und
Spade-Code erzeugen kommen in späteren Etappen.

Für den C++17-Einstieg gibt es die
[begleitende Kurzeinführung](cpp17_einstieg.md).

## 2. Die feste CPU-v2-ISA

Die CPU-Definition bleibt unverändert:
[ISA](../cpu_v2/isa.md) und [ROM-Quellcode](../../src/cpu_v2/rom16x8.spade).

```text
  7 6 5 4   3 2 1 0
+---------+---------+
| Opcode  | Operand |
+---------+---------+
    4 Bit     4 Bit
```

| Befehl | Opcode (hex) | Operand | Beispiel |
|---|---:|---|---|
| ADD | 0 | 0–15 | `ADD 15` → `0x0F` |
| AND | 1 | 0–15 | `AND 10` → `0x1A` |
| OR | 2 | 0–15 | `OR 15` → `0x2F` |
| XOR | 3 | 0–15 | `XOR 15` → `0x3F` |
| LDI | 4 | 0–15 | `LDI 5` → `0x45` |
| JMP | 8 | absolute Adresse | `JMP 1` → `0x81` |
| JZ | 9 | absolute Adresse | `JZ 5` → `0x95` |
| NOP | C | ohne Quelltextoperand | `NOP` → `0xC0` |

Die reservierten Opcodes gehören **nicht** zur Sprache des Assemblers.

Das Maschinenwort ergibt sich aus dem Opcode im oberen und dem
Immediate im unteren Halbbyte. Beide Felder müssen vollständig
erhalten bleiben. Das Zusammenführen erfolgt mit einer
Linksverschiebung und einer bitweisen ODER-Verknüpfung.

### Unterschied zwischen Encoder und späterem Parser

`encode(Opcode, unsigned)` ist eine **niedrige Ebene**: Sie packt
zwei 4-Bit-Felder zu einem Byte. Deshalb wird sie auch für
`Opcode::NOP` mit jedem Wert von 0 bis 15 getestet. Das erzeugt
unterschiedliche Bitmuster, obwohl die CPU die unteren Bits eines
NOP nicht auswertet.

Erst der spätere **Parser** macht die benutzerfreundliche Regel:
`NOP` hat keinen Operanden und wird stets als `NOP 0`, also
`0xC0`, kodiert.

## 3. ROM-Grenzen nicht vergessen

Die Hardware besitzt einen **4-Bit-Programmzähler** und
ein **16×8-Bit-ROM**. Das bedeutet:

| Situation | Vorgabe für den späteren Assembler |
|---|---|
| 0 Befehle | zulässig, alle Adressen ergeben NOP |
| 1–15 Befehle | fehlende Adressen ergeben NOP |
| genau 16 Befehle | zulässig, ROM vollständig belegt |
| 17 oder mehr Befehle | Fehler, kein Abschneiden |
| Operand oder Sprungziel 16 | Fehler, außerhalb von 4 Bit |
| Ziel einer Marke hinter Adresse 15 | Fehler, auch wenn die Marke syntaktisch existiert |

Der **Encoder prüft jetzt nur die Operandenbreite**, nicht die
Programmgröße. Die Konstante `kRomWords = 16` steht bereits
in `encoder.hpp` und wird später beim Zusammenbauen des ganzen
Programms verwendet. Die Kapazitätsprüfung darf nicht mit einer
Bitmaske oder einem Modulo „repariert“ werden: Das würde Fehler
verstecken.

Der spätere Generator soll die überzähligen ROM-Adressen durch
einen Standardarm füllen:

```spade
_ => 0xC0, // Nicht belegte Adressen: NOP
```

## 4. Deine C++17-Aufgabe

Datei: `logic_basics/assembler/encoder.cpp`

Die öffentliche Schnittstelle ist bereits in
[`encoder.hpp`](../../assembler/encoder.hpp) festgelegt:

```cpp
std::uint8_t encode(Opcode opcode, unsigned immediate);
```

1. Prüfe, ob `immediate` zwischen 0 und 15 liegt.
2. Wirf andernfalls `std::out_of_range` aus `<stdexcept>`.
3. Kombiniere die beiden Halbbytes zu einem `std::uint8_t`.
4. Ändere weder den Opcode-Enum noch die vorgegebenen Testwerte.

Du brauchst noch keine Schleife, Datei-Ein-/Ausgabe oder
Labeltabelle. Den umgebenden Namespace enthält die Datei bereits.

Ein Mini-Beispiel, das **nicht** Teil unserer ISA ist:

```cpp
unsigned feld = 0b10u;
unsigned nummer = 0b01u;
unsigned zusammen = (feld << 2) | nummer; // 0b1001 = 9
```

## 5. Testen

Im Repository-Root und in der Nix-Dev-Shell:

```sh
make test_assembler_encoder
```

Die Testbench prüft **8 definierte Opcodes × 16 erlaubte Operanden**
sowie drei ungültige Werte (16, 17 und 256). Das sind
**131 Vergleiche**. Bei einem unerlaubten Wert muss
`std::out_of_range` geworfen werden.

Zu Beginn soll der Test absichtlich fehlschlagen, denn die Funktion
liefert im Platzhalter noch `0` zurück. Die bisherigen Hardwaretests
laufen unabhängig weiter mit `make test_completed`.

`make assembler` kompiliert bereits das spätere Kommandozeilenprogramm.
Es verarbeitet noch keine Eingaben und meldet das entsprechend.
**Die offene Encoder-Aufgabe wird nicht automatisch in
`make test` aufgenommen**, damit die abgeschlossene Hardware-Regression
als Kontrollgruppe grün bleibt.

## 6. Was danach kommt

Nach einem erfolgreichen Encoder-Test folgen in getrennten Schritten
die Erkennung von Befehlszeilen und Kommentaren, die Ermittlung
von Labeladressen, die Prüfung der ROM-Größe und schließlich
die Ausgabe des vollständigen Spade-`match`-Blocks.

Das [Countdown-Beispiel](../../assembler/examples/countdown.asm)
ist bereits als spätere Eingabedatei vorhanden; es ist mit zehn
Befehlen kleiner als das ROM.

---

[← Zurück: CPU-v2-Integration](../cpu_v2/cpu_v2.md) · [Zur Kapitelübersicht](../README.md)
