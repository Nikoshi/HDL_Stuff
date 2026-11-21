# Architektur-Spickblatt

Kurzübersicht der vorhandenen Hardware. Details stehen in den jeweiligen Lernkapiteln.

## Vom Gatter zur CPU

| Gruppe | Bausteine | Quellcode |
|---|---|---|
| Grundlagen | Multiplexer, Halb-/Volladdierer, 2-/4-Bit-Addierer | [`foundations/`](../src/foundations/) |
| Sequentielle Logik | 1-/4-Bit-Register, Zähler, Schieberegister | [`foundations/`](../src/foundations/) |
| Zustandsautomaten | Vierphasige Ampel-FSM | [`fsm.spade`](../src/foundations/fsm.spade) |
| Rechenwerk | 4-Bit-ALU: ADD, AND, OR, XOR | [`alu.spade`](../src/cpu/alu.spade) |
| ALU-Vergleich | Selbst gebauter Ripple-Carry-Addierer | [`alu_ripple.spade`](../src/foundations/alu_ripple.spade) |

## CPU v1: 4-Bit-Daten, 6-Bit-Befehle

```text
PC (4 Bit) → ROM 16×6 → Decoder6 → Akkumulator4
    ↑                                   │
    └── PC zählt weiter / externer Sprung
                         Akkumulator4 = ALU + ACC-Register
```

| Baustein | Aufgabe | Spade-Datei |
|---|---|---|
| `pc4_top` | Adresse speichern, zählen, extern springen | [`pc4.spade`](../src/cpu/pc4.spade) |
| `rom16x6_top` | Aus 16 Adressen einen festen 6-Bit-Befehl lesen | [`rom16x6.spade`](../src/cpu/rom16x6.spade) |
| `decoder6_top` | 2-Bit-ALU-Operation und 4-Bit-Operand trennen | [`decoder6.spade`](../src/cpu/decoder6.spade) |
| `accumulator4_top` | ALU-Ergebnis in 4-Bit-ACC übernehmen | [`accumulator.spade`](../src/cpu/accumulator.spade) |
| `instruction_exec6_top` | Decoder und Akkumulator verbinden | [`instruction_exec6.spade`](../src/cpu/instruction_exec6.spade) |
| `cpu4_top` | PC, ROM und Ausführungsstufe verbinden | [`cpu4.spade`](../src/cpu/cpu4.spade) |

Die vier Operationen sind ADD, AND, OR, XOR. Sprünge kommen
hier **von außen** über `jump` und `target`, nicht aus dem Maschinenwort.

## CPU v2: 4-Bit-Daten, 8-Bit-Befehle

```text
             ┌─────────────────────────────────────────────┐
             │                                             │
PC (4 Bit) ──┴→ ROM16x8 → Decoder8 → Steuerfunktion        │
              (8 Bit)   ↙ Opcode  ↘ Immediate             │
                              │           │                │
                              ▼           ▼                │
                           Steuerung   ALU4 ← ACC (4 Bit)   │
                              │           │                │
                              └─────┬─────┘                │
                                    ▼                      │
                               state_v2                    │
                              PC-/ACC-Register ────────────┘
```

| Baustein | Aufgabe | Spade-Datei |
|---|---|---|
| `rom16x8_top` | Festes Countdown-/LED-Programm | [`rom16x8.spade`](../src/cpu_v2/rom16x8.spade) |
| `decoder8_top` | Opcode (4 Bit) und Immediate (4 Bit) trennen | [`decoder8.spade`](../src/cpu_v2/decoder8.spade) |
| `decode_control_signals` | ACC-Schreiben, Datenquelle, ALU-Op, PC-Modus wählen | [`control8.spade`](../src/cpu_v2/control8.spade) |
| `alu4_top` | Gemeinsame 4-Bit-ALU aus CPU v1 | [`alu.spade`](../src/cpu/alu.spade) |
| `cpu_v2_state_top` | PC und ACC gemeinsam takten; bedingte Sprünge | [`state_v2.spade`](../src/cpu_v2/state_v2.spade) |
| `cpu_v2_top` | Die fünf Teile verschalten | [`cpu_v2.spade`](../src/cpu_v2/cpu_v2.spade) |

Neue Befehle gegenüber v1: `LDI`, `JMP`, `JZ`, `NOP`.
Der Sprung kommt aus dem **Maschinenbefehl**, nicht von einem
externen `jump`-Eingang. Die ALU ist kombinatorisch;
**Register liegen nur im Zustandsblock**.

## Signale kurz übersetzt

| Signal | Bedeutung |
|---|---|
| `pc` | Gespeicherte Adresse des nächsten Befehls |
| `instruction` | Das aktuell bei `ROM[pc]` anliegende Maschinenwort |
| `opcode` / `immediate` | Befehlsart / 4-Bit-Operand oder Sprungziel |
| `alu_op` | 2-Bit-Auswahl ADD, AND, OR oder XOR |
| `acc` / `alu_result` | Gespeicherter Wert / kombinatorisch berechneter Wert |
| `acc_write` / `acc_from_imm` | ACC-Schreibfreigabe / direkte Immediate-Übernahme |
| `pc_mode` | Weiterzählen, JMP oder JZ |
| `rst` / `advance` | Synchron zurücksetzen / Verarbeitung eines Takts freigeben |

**Zeitregel:** An der steigenden Flanke werden die Folgezustände
aus dem **alten PC und ACC** übernommen. Direkt danach liefert das
kombinatorische ROM schon das Wort der **neuen PC-Adresse**.

## Schnelle Befehle

```sh
make test_cpu4       # Integrierte CPU v1
make test_cpu_v2     # Integrierte CPU v2
make test_completed  # Alle 23 abgeschlossenen Hardwaretests
make demo_cpu_v2     # CPU-v2-Softwaremodell ohne Hardware
```

Detaillierte Spezifikation: [CPU-v2-ISA](cpu_v2/isa.md) ·
[Kapitel IX: Integration](cpu_v2/cpu_v2.md).

---

[← Zur Projektübersicht](../../README.md)
