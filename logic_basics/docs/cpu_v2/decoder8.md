# CPU v2, Kapitel V: Felddecoder und Steuerbefehle (abgeschlossen)

**Status:** Abgeschlossen. Der lokale Testlauf wurde mit **384/384 Vergleichen bestanden** gemeldet. Die erste CPU-Generation bleibt unverändert.

## 1. Lernziel: Zwei Aufgaben nicht vermischen

Der 8-Bit-Befehl wird in zwei logisch unterschiedliche Schritte zerlegt:

```text
  ROM liefert 8-Bit-Befehl
              |
              v
    +------------------+
    | decoder8_top     |  nur Bitfelder trennen
    +------------------+
       |          |
     opcode     immediate
       |          |
       v          +---------------> ALU-Operand / Sprungadresse
   Steuerlogik
       |
       +----> ACC schreiben? Welchen Wert?
       +----> PC + 1, JMP oder JZ?
```

**Diese Übung implementiert ausschließlich den Felddecoder.**
Eine spätere `control8`-Entity wird den Opcode interpretieren.
Das macht die Bausteine unabhängig testbar und verhindert, dass
wir den alten 6-Bit-Decoder oder die CPU v1 umbauen müssen.

## 2. Die verbindliche `decoder8_top`-Schnittstelle

Datei: `logic_basics/src/cpu_v2/decoder8.spade`

| Port | Typ | Bedeutung |
|---|---|---|
| `instruction` | `uint<8>` | Vollständiges Maschinenwort |
| `opcode` | `inv uint<4>` | Höherwertiges Nibble, Bits 7 bis 4 |
| `immediate` | `inv uint<4>` | Niederwertiges Nibble, Bits 3 bis 0 |

Der Decoder ist **rein kombinatorisch**: kein `clk`, kein
`rst`, kein `reg` und kein zusätzliches Steuerbit.

```text
instruction: [ b7 b6 b5 b4 | b3 b2 b1 b0 ]
opcode:      [ b7 b6 b5 b4 ]
immediate:   [ b3 b2 b1 b0 ]
```

Beispiele:

| Eingabe | Opcode | Immediate | Lesart |
|---|---|---|---|
| `0x45` | `0x4` | `0x5` | LDI 5 |
| `0x95` | `0x9` | `0x5` | JZ 5 |
| `0x0F` | `0x0` | `0xF` | ADD 15 |
| `0xC0` | `0xC` | `0x0` | NOP |
| `0xFF` | `0xF` | `0xF` | Reserviert, aber Bitfelder unverändert ausgeben |

**Auch reservierte Opcodes werden korrekt zerlegt.**
Ob die CPU sie später ausführen darf und was dann geschieht,
entscheidet *nicht* der Felddecoder.

### Spade-Vertiefung: Bitreihenfolge

Das bereits erprobte `decoder6.spade` ist eine gute Referenz
für `.as_bits()` und `.as_uint()`.

Ein Ausdruck wie `instruction.as_bits()` liefert ein Array von
Einzelbits mit `bits[0]` als **niederwertigstem** Bit.
Bei vier ausgewählten Bits ist ihre Reihenfolge im neuen
Array ebenfalls entscheidend: Dessen erstes Element wird
nach `.as_uint()` zum niederwertigsten Bit des Ergebnisses.

**Denkaufgabe:** Welche Arrayindizes bilden den Opcode, und
welche das Immediate? Kontrolliere deine Überlegung an `0x95`
und `0x4F`, bevor du den Stub implementierst.

## 3. Vorgeschlagener Vertrag für die spätere Steuerlogik

Die folgende Tabelle ist unsere geplante **Steuersignalspezifikation**,
noch keine implementierte Entity. Die Steuerlogik erhält später
den 4-Bit-`opcode` aus dem Felddecoder.

Wir verwenden wenige Signale und verzichten vorerst auf einen
aufwendigen hierarchischen Gruppendecoder:

- `acc_write: bool`: Darf bei einem aktiven CPU-Schritt der ACC aktualisiert werden?
- `acc_from_imm: bool`: Kommt der neue ACC-Wert direkt aus dem Immediate statt aus der ALU?
- `alu_op: uint<2>`: Wählt ADD=00, AND=01, OR=10 oder XOR=11 für die vorhandene ALU.
- `pc_mode: uint<2>`: `00` = PC+1, `01` = unbedingter JMP, `10` = JZ, `11` = reserviert.

| Opcode | Befehl | `acc_write` | `acc_from_imm` | `alu_op` | `pc_mode` |
|---|---|---:|---:|---|---|
| `0000` | ADD | 1 | 0 | `00` | `00` |
| `0001` | AND | 1 | 0 | `01` | `00` |
| `0010` | OR | 1 | 0 | `10` | `00` |
| `0011` | XOR | 1 | 0 | `11` | `00` |
| `0100` | LDI | 1 | 1 | `00` | `00` |
| `1000` | JMP | 0 | 0 | `00` | `01` |
| `1001` | JZ | 0 | 0 | `00` | `10` |
| `1100` | NOP | 0 | 0 | `00` | `00` |

Für Nicht-ALU-Befehle ist `alu_op=00` ein **harmloser
Platzhalterwert**. Er bedeutet dort nicht, dass ADD ausgeführt
oder der ACC beschrieben wird. Entscheidend ist `acc_write`.

`acc_from_imm` wird nur bei LDI benötigt. Bei JMP/JZ
verwendet dagegen die **PC-Steuerung** das Immediate als Adresse.

Für die nächste Stufe haben wir uns auf einen **sicheren Standard** geeinigt: Die acht reservierten Opcodes erzeugen dieselben Steuersignale wie NOP (`acc_write=0`, `acc_from_imm=0`, `alu_op=00`, `pc_mode=00`). Sie bleiben für spätere ISA-Erweiterungen reserviert, führen in der jetzigen Hardware aber keine Zustandsänderung am ACC aus; der PC zählt normal weiter. Siehe [Kapitel VI](control8.md).

### Was gehört nicht in `control8`?

Die Zustandsregister müssen zusätzlich die bereits bekannten
globalen Signale berücksichtigen:

- `rst` setzt PC und ACC synchron auf 0.
- `advance=0` hält PC und ACC unabhängig vom Opcode an.
- Bei `pc_mode=10` wird der **alte** ACC-Wert gegen null
  geprüft, erst dann die nächste PC-Adresse ausgewählt.

Diese Entscheidungen gehören zur Register-/PC-Verdrahtung,
nicht in den reinen Felddecoder.

## 4. Eigene Spade-Aufgabe

Die fertig implementierte `decoder8_top` verwendet `as_bits()` und
`as_uint()`, um die beiden Nibbles ohne weitere Logik auszugeben.
Als nächste Übung folgt die getrennte Steuerlogik `control8`.

## 5. TDD und Simulation

Im Repository-Root:

```sh
make test_decoder8
```

Der unabhängige C++-Test `logic_basics/test/decoder8.cc` prüft:

- Alle **256** möglichen 8-Bit-Eingaben gegen `(instruction >> 4) & 15`
  und `instruction & 15`.
- **128** weitere nicht-monotone Eingabewechsel ohne Takt,
  um unerwünschte gespeicherte Zustände aufzudecken.

Insgesamt: **384 Vergleiche**. Der frühere Null-Platzhalter wurde
ersetzt; der Nutzer hat **384/384 bestandene Tests** gemeldet.

VCD-Trace: `logic_basics/build/decoder8.vcd`.

`make test_completed` enthält jetzt auch den abgeschlossenen `decoder8`-Test.
Die nächste offene Aufgabe (`control8`) läuft über `make test_control8`.

Für unser Countdown-/LED-Alarm-Programm bleibt zusätzlich
`make demo_cpu_v2` als getrenntes Softwaremodell verfügbar.

## 6. Ausblick

Als nächstes implementieren und testen wir die
[Steuerlogik `control8`](control8.md). Erst danach
verbinden wir ihre Signale mit ACC und PC.

Das vollständige Demo-Programm steht im
[ISA-Entwurf](isa.md#5-isa-praxistest-countdown-mit-led-alarm).
