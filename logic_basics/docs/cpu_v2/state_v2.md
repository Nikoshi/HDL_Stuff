# CPU v2, Kapitel VII: Zustandssteuerung für PC und ACC

**Status:** Offene Spade-TDD-Übung. Felddecoder und Opcode-Steuerfunktion sind abgeschlossen.

## 1. Lernziel

Bisher haben wir zwei rein kombinatorische Bausteine:

- `decoder8_top` trennt das 8-Bit-Wort in Opcode und Immediate.
- `decode_control_signals` liefert daraus Schreibfreigabe, Datenquelle, ALU-Operation und PC-Modus.

Nun kommt der Schritt, in dem **zwei Register tatsächlich ihren Zustand ändern**.
Wir wollen gezielt nur das Speichern und die Auswahl der nächsten Werte
bauen. Ein neuer ROM und der endgültige CPU-v2-Top-Level sind noch nicht nötig.

```text
                immediate ────────────────┬─────────────┐
                                         │             │
  ALU-Ergebnis ───────┐                  │             │
                      v                  v             v
  acc_from_imm ───> ACC-Mux ────────> ACC-Register     |
  acc_write ────────────────────────>   (4 Bit)        |
                                         |             |
                                         └─> ACC == 0  |
                                               |       |
                     pc_mode ──────────────────┼───────┤
                                               v       v
                                               PC-Auswahl
                                                   |
                                              PC-Register
                                                (4 Bit)
```

Für diese Übung erhält die neue Entity **bereits berechnete
Steuersignale** und ein **extern bereitgestelltes ALU-Ergebnis**.
Die bestehende ALU muss daher hier noch nicht instanziiert werden.

## 2. Die verbindliche Schnittstelle

Datei: `logic_basics/src/cpu_v2/state_v2.spade`

Top-Level für die unabhängige Verilator-Testbench:
`cpu_v2_state_top`.

| Signal | Typ | Bedeutung |
|---|---|---|
| `clk` | `clock` | Gemeinsamer Takt |
| `rst` | `bool` | Synchroner Reset beider Register |
| `advance` | `bool` | Gemeinsames Schritt-/Pause-Signal |
| `immediate` | `uint<4>` | Datenkonstante oder Sprungziel |
| `alu_result` | `uint<4>` | Bereits kombinatorisch berechnetes ALU-Ergebnis |
| `acc_write` | `bool` | Schreibfreigabe des ACC bei aktivem Schritt |
| `acc_from_imm` | `bool` | Wähle Immediate statt ALU-Ergebnis |
| `pc_mode` | `uint<2>` | `00` normal, `01` JMP, `10` JZ, `11` Fallback |
| `pc` | `inv uint<4>` | Gespeicherter PC |
| `acc` | `inv uint<4>` | Gespeicherter Akkumulator |

Die beiden Ausgänge zeigen die **gespeicherten Werte nach der letzten
steigenden Taktflanke**, nicht die aktuell berechneten D-Eingänge.

## 3. Zustandsregeln und Signalpriorität

Seien `PC_alt` und `ACC_alt` die vor einer steigenden Flanke
gespeicherten Werte. Dann gelten folgende Regeln in genau dieser Priorität:

1. **`rst=1`:** PC und ACC werden beide auf 0 gesetzt, auch bei `advance=0`.
2. **`rst=0, advance=0`:** Beide Register halten ihren Zustand, unabhängig von allen anderen Eingängen.
3. **`rst=0, advance=1`:** PC und ACC übernehmen gleichzeitig die aus ihren alten Zuständen berechneten neuen Werte.

### Der Akkumulator

```text
ACC_neu =
    0               bei rst
    ACC_alt         bei Pause
    ACC_alt         wenn acc_write = 0
    immediate       wenn acc_write = 1 und acc_from_imm = 1
    alu_result      wenn acc_write = 1 und acc_from_imm = 0
```

Die eigentliche ALU-Operation gehört in die spätere
Ausführungsstufe. Ein `JMP` oder `JZ` muss den ACC
nicht verändern.

### Der Programmzähler

Wenn die CPU nicht gerade zurückgesetzt oder angehalten ist:

| `pc_mode` | Bedingung | `PC_neu` |
|---|---|---|
| `00` | immer | `(PC_alt + 1) mod 16` |
| `01` | immer | `immediate` |
| `10` | `ACC_alt == 0` | `immediate` |
| `10` | `ACC_alt != 0` | `(PC_alt + 1) mod 16` |
| `11` | immer | `(PC_alt + 1) mod 16` (sicherer Fallback) |

**Die JZ-Falle:** Ist `acc_write=1` und `pc_mode=10` gleichzeitig
aktiv, darf die Entscheidung über den Sprung nur **ACC_alt** lesen.
Die Testbench prüft solche Kombinationen bewusst, auch wenn die aktuelle
Opcode-Steuerfunktion sie im normalen Betrieb nicht erzeugt.

`pc_mode=11` ist keine neue ISA-Operation. Die sequentielle Fortsetzung
ist lediglich das definierte, sichere Verhalten für diesen sonst
unbenutzten Steuerwert.

## 4. Warum hier eine Entity statt einer Funktion?

Eine reine `fn` ist passend für unseren `control8`-Decoder.
Hier werden dagegen **zwei echte Register** über steigende
Taktflanken hinweg gespeichert. Deshalb ist eine `entity` mit
`clk` und `reg(clk)` sinnvoll.

Spade-Muster, das du bereits kennst:

```spade
reg(clk) stored: uint<4> reset(rst: 0) =
    if enable { next_value } else { stored };
```

`next_value` ist hier **nur ein Platzhaltername**, keine
vorgegebene Lösung für die PC- oder ACC-Auswahl.

Wenn der ACC einen neuen Wert übernimmt, muss er zwischen
`immediate` und `alu_result` wählen. Wenn der PC springt,
muss er zwischen dem hochgezählten PC und einer Adresse
wählen. Diese Multiplexer sind der neue Lerninhalt.

## 5. Deine Spade-Aufgabe

Der vorhandene Stub in `state_v2.spade` implementiert Reset,
Pause und einige Standardfälle, aber noch nicht die vollständige
ACC-Datenquellenwahl und nicht die Sprungmodi.

Ergänze **nur diese Datei**, ohne den funktionierenden
`decoder8`, `control8` oder CPU v1 umzuschreiben.
Für die spätere Integration bleiben die Steuersignale
bewusst als Eingangsports sichtbar.

Tipp: Beginne mit einer separaten Überlegung für
`PC_neu` und `ACC_neu`. Erst danach wird festgelegt,
wie ihre jeweiligen `reg(clk)`-Anweisungen aussehen.

## 6. Testbench und erwarteter Umfang

Im Repository-Root:

```sh
make test_state_v2
```

Die unabhängige C++-Testbench
`logic_basics/test/state_v2.cc` prüft:

- Synchronen Reset und Pause, auch bei widersprüchlichen Steuersignalen.
- Alle **256** Kombinationen aus initialem PC und ACC (jeweils 0–15).
- Für jeden Anfangszustand alle vier PC-Modi und beide ACC-Steuerbits.
- Unterschiedliche ALU- und Immediate-Werte und die korrekte JZ-Bedingung.
- Eingangsänderungen bei hohem Takt und fallende Flanken ohne Zustandsänderung.
- Eine gemischte Folge mit längeren Abschnitten ohne Reset.

**Erwartet: 69.635 Zustandsvergleiche.** Der erste Eingangswechsel vor
der initialen Reset-Flanke wird nicht verglichen, weil die Register
zu diesem Zeitpunkt noch keinen definierten Zustand besitzen.

Die erwarteten Zustände berechnet ein eigenständiges Softwaremodell.
Bei einem Fehler gibt die Testbench die Eingänge sowie
`(PC, ACC)` aus. Die VCD-Datei liegt unter
`logic_basics/build/state_v2.vcd`.

**Die Vorlage soll zunächst fehlschlagen.**
`make test_completed` prüft separat die 20 bereits
abgeschlossenen Hardware-Tests; `make test` umfasst
zusätzlich diese neue Aufgabe.

## 7. Nächster Baustein

Sobald dieser Zustandsblock korrekt ist, kombinieren wir
`decoder8`, `decode_control_signals`, die bestehende ALU
und die neuen Register. Anschließend erhält CPU v2 einen
16×8-Bit-ROM mit unserem Countdown-/LED-Alarm-Programm.

Weiterführend: [ISA](isa.md), [Kapitel VI](control8.md).
