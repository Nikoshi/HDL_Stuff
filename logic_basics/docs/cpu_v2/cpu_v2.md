# CPU v2, Kapitel IX: Die vollständige CPU verbinden

**Status:** Abgeschlossen. Die integrierte CPU v2 bestand im gemeldeten lokalen Lauf **3.691/3.691 Zustandsvergleiche**.

## 1. Was du in diesem Kapitel baust

Die fünf benötigten Bausteine funktionieren bereits einzeln. Jetzt werden
sie zu **einem gemeinsamen Datenpfad** verbunden:

- Der gespeicherte **PC** wählt ein Maschinenwort im ROM.
- Der Decoder teilt das Wort in **Opcode** und **Immediate**.
- Die Steuerfunktion erzeugt die Signale für ALU und Register.
- Die ALU rechnet mit dem **gespeicherten ACC** und dem Immediate.
- Der Zustandsblock übernimmt an der nächsten steigenden Flanke
  den neuen PC und ACC.

**Hier wird keine neue ISA und keine neue Registerlogik implementiert.**
Die Aufgabe ist die Verdrahtung der vorhandenen Module. Die Spade-Dateien
der Einzelbausteine bleiben unverändert.

## 2. Die konkrete Schaltung

```text
                         ┌────────────┐
                 PC ────>│ ROM 16×8   │──────> instruction
                 ▲       └────────────┘            │
                 │                                 ▼
                 │                           ┌───────────┐
                 │                           │ decoder8  │
                 │                           └───────────┘
                 │                             │       │
                 │                         opcode   immediate
                 │                             │       │
                 │                             ▼       ├────────────┐
                 │                         control8    │            │
                 │                             │       │            │
                 │                    alu_op, acc_write│            │
                 │                    acc_from_imm,    │            │
                 │                    pc_mode          ▼            │
                 │                             │   ┌────────┐       │
                 │                             └──>│ ALU4   │<── ACC│
                 │                                 └────────┘       │
                 │                                      │           │
                 │                                  alu_result      │
                 │                                      │           │
                 │                                      ▼           ▼
                 │                             ┌─────────────────────┐
                 └─────────────────────────────│ cpu_v2_state_top    │
                                               │ (PC- und ACC-Reg.)  │
                                               └─────────────────────┘
                                                          │
                                                         ACC
```

Der ROM und die anderen kombinatorischen Bausteine hängen am gespeicherten
Zustand. Dadurch wird jeweils **ROM[PC_alt]** mit **ACC_alt** ausgeführt.
An der Flanke aktualisieren sich beide Register gemeinsam.

### Vorhandene Module

| Modul | Datei | Rolle |
|---|---|---|
| `rom16x8_top` | `rom16x8.spade` | `PC → instruction` |
| `decoder8_top` | `decoder8.spade` | `instruction → (opcode, immediate)` |
| `decode_control_signals` | `control8.spade` | `opcode → (acc_write, acc_from_imm, alu_op, pc_mode)` |
| `alu4_top` | `../cpu/alu.spade` | `(ACC, immediate, alu_op) → alu_result` |
| `cpu_v2_state_top` | `state_v2.spade` | Speichert PC und ACC |

**Achtung bei `control8`:** `decode_control_signals` ist eine
**`fn`**, keine `entity`. Sie wird wie eine Funktion aufgerufen.
`test_control8_top` ist nur die Verilator-Testhülle und gehört
nicht in den CPU-Datenpfad.

## 3. Spade-Ports richtig verbinden

Ein Ausgang vom Typ `inv uint<4>` ist kein gewöhnlicher Wert, den
du direkt weiterreichen kannst. Für solche Verbindungen kennen wir
aus `cpu4.spade` das Muster mit `port()`.

Dieses **Teilbeispiel** zeigt nur den ROM-Anschluss.
`current_pc` muss an anderer Stelle als lesbares Signal aus dem
Zustandsblock kommen; es ist hier absichtlich noch nicht definiert:

```spade
let (current_instruction, instruction_wire) = port();

let _ = inst rom16x8::rom16x8_top {
    address: current_pc,
    instruction: instruction_wire
};
```

- `instruction_wire` wird mit dem `inv`-Ausgang des ROM verbunden.
- `current_instruction` ist die lesbare Seite für Decoder und Top-Level.
- `port()` selbst erzeugt kein Register und fügt keine Taktflanke hinzu.

Plane die benötigten Verbindungen am besten erst mit einer kleinen
Liste: **Welcher Baustein produziert das Signal, wer liest es?**
Dabei helfen Namen wie `current_pc`, `current_acc`,
`current_instruction`, `opcode` und `immediate`.

Die wiederverwendete ALU besitzt außerdem die Ausgänge
`carry_out` und `zero`. Diese gehören zu ihrer Schnittstelle,
werden in CPU v2 aber nicht für die Sprungbedingung benötigt.
**JZ prüft den alten ACC im Zustandsblock**, nicht das
`zero`-Signal des aktuellen ALU-Ergebnisses.

### Das Muster für den PC und den ACC

Die lesbare Seite eines `port()`-Paares kann vor dem `inst`
verwendet werden, das den passenden `inv`-Ausgang anschließt:

```spade
let (current_pc, pc_wire) = port();
```

`pc_wire` gehört später an den Ausgang `pc` der
`cpu_v2_state_top`-Entity. `current_pc` ist zugleich
der Eingang `address` des ROM und der beobachtbare Top-Level-PC.
Entsprechend benötigt der gespeicherte ACC eine Verbindung zum
ALU-Eingang. Das ist eine **Rückkopplung über Register**:
Die Register entkoppeln die alte von der neuen Taktperiode.

Eine hilfreiche Verbindungsliste für die eigene Verdrahtung:

| Lesbares Signal | Ursprung | Wird gebraucht von |
|---|---|---|
| `current_pc` | `cpu_v2_state_top.pc` | ROM, Top-Level-Ausgang |
| `current_instruction` | `rom16x8_top.instruction` | Decoder, Top-Level-Ausgang |
| `current_opcode` | `decoder8_top.opcode` | `decode_control_signals` |
| `current_immediate` | `decoder8_top.immediate` | ALU und Zustandsblock |
| `current_acc` | `cpu_v2_state_top.acc` | ALU, Top-Level-Ausgang |
| `alu_result` | `alu4_top.result` | Zustandsblock |

### Achtung: Die ALU ist nicht die Ausführungsstufe

Die vorhandene `alu::alu4_top`-Entity hat die Eingänge
`a: uint<4>`, `b: uint<4>`, `op: uint<2>` und
die drei `inv`-Ausgänge `result: uint<4>`,
`carry_out: bool`, `zero: bool`.
Es gibt **keine Ports** namens `clk`, `rst`, `enable`,
`decoded_operand` oder `decoded_op`.

Der ALU-Operationseingang erhält **`alu_op`** aus der
Steuerfunktion, nicht den vollständigen vier Bit breiten Opcode.
Das Ergebnis braucht ein eigenes `port()`-Paar, damit es
als lesbarer Wert an die Zustandsregister weitergegeben werden kann.
Auch die beiden hier unbenutzten `inv`-Ausgänge `carry_out`
und `zero` müssen beim Instanziieren passende Anschlüsse erhalten.
Ihr Wert entscheidet nicht über `JZ`.

### Importpfade und Projektprüfung

Die Module sind in `src/main.spade` und
`src/cpu_v2/main.spade` registriert.
In einer anderen Datei werden sie deshalb z. B. mit
`use lib::cpu_v2::rom16x8;` importiert;
die gemeinsame ALU liegt unter `lib::cpu::alu`.
`lib` verweist hier auf das aktuelle Swim-Projekt.

Falls der Editor einen gültigen Modulpfad dennoch als unbekannt
markiert, zunächst den Compiler als Vergleich heranziehen:

```sh
nix develop -c sh -c 'cd logic_basics && swim build'
```

Bei erfolgreichem Build den Spade-LSP neu starten.
Ein LSP-Fehler allein beweist noch keinen ungültigen Import.

## 4. Verbindliche Top-Level-Schnittstelle

Datei: `logic_basics/src/cpu_v2/cpu_v2.spade`

```spade
#[no_mangle(all)]
pub entity cpu_v2_top(
    clk: clock,
    rst: bool,
    advance: bool,
    pc: inv uint<4>,
    instruction: inv uint<8>,
    acc: inv uint<4>
)
```

Die sichtbaren Ausgänge bedeuten:

| Ausgang | Bedeutung |
|---|---|
| `pc` | Gespeicherte Adresse nach der letzten steigenden Flanke |
| `acc` | Gespeicherter Datenwert nach der letzten steigenden Flanke |
| `instruction` | **Momentan** aus `ROM[pc]` gelesenes Maschinenwort |

`rst` und `advance` werden an den vorhandenen Zustandsblock
weitergegeben. Eine neue Registerstufe ist nicht vorgesehen.

Die Implementierung verbindet die vorhandenen Bausteine ohne zusätzliche
Register. Die drei Top-Level-Ausgänge werden aus den aktuellen PC-,
ACC- und ROM-Signalen gespeist.

## 5. Ein vollständiger Ablauf über vier Takte

Nach einem synchronen Reset gilt `PC=0` und `ACC=0`.
Der kombinatorische ROM zeigt bereits `0x45` für `LDI 5`.

| Steigende Flanke | Ausgeführter Befehl | PC danach | ACC danach | Jetzt sichtbarer ROM-Befehl |
|---:|---|---:|---:|---|
| Reset | keiner | 0 | 0 | `0x45` (LDI 5) |
| 1 | LDI 5 | 1 | 5 | `0x0F` (ADD 15) |
| 2 | ADD 15 | 2 | 4 | `0x95` (JZ 5) |
| 3 | JZ 5, nicht genommen | 3 | 4 | `0x81` (JMP 1) |
| 4 | JMP 1 | 1 | 4 | `0x0F` (ADD 15) |

**Wichtig für die Testbench:** Nach der Flanke steht am Ausgang
`instruction` schon der Befehl des **neuen** PC. Er ist nicht
automatisch das Maschinenwort, das gerade ausgeführt wurde.

Ein `advance=0` hält beide Register; ein synchroner Reset
setzt sie auf null, auch während einer Pause.

## 6. Testvertrag

Im Repository-Root:

```sh
make test_cpu_v2
```

Die C++-Testbench `logic_basics/test/cpu_v2/cpu_v2.cc` verwendet ein
unabhängiges ISA-Softwaremodell und prüft **PC, ACC und das aktuelle
ROM-Wort nach jeder Beobachtung**:

- Drei vollständige 20-Schritt-Durchläufe des Countdown-/LED-Programms.
- Zusätzliche Pausen innerhalb dieser Abläufe.
- Reset nach unterschiedlich langen Teilabläufen, auch bei `advance=0`.
- Eingangsänderungen bei hohem Takt und fallende Flanken ohne Zustandswechsel.
- Eine längere deterministische Folge mit Reset und Pausen.

**Erwartet sind 3.691 Zustandsvergleiche.** Der erste
Eingangswechsel vor dem initialen Reset zählt nicht.
Trace: `logic_basics/build/cpu_v2.vcd`.

Die Testbench wurde mit dem fertigen Top-Level lokal vollständig bestanden
(3.691/3.691 gemeldete Vergleiche). `make test_completed` enthält jetzt
**23 abgeschlossene Hardware-Testprogramme**, einschließlich CPU v2;
`make test` führt dieselbe Regression aus.

Die ROM-Adressen 4 und 10–15 werden durch das festgelegte Programm
normalerweise nicht erreicht. Ihre Inhalte sind bereits unabhängig
im Test `test_rom16x8` geprüft worden.

## 7. Nach diesem Kapitel

Der erfolgreiche CPU-v2-Integrationstest schließt die Hardware
dieser CPU-Generation ab. Danach eignet sich ein
kleines **Assembler-Projekt** als Softwareübung: symbolische
Maschinenbefehle einlesen und daraus den `match address`-Block
für ein Spade-ROM erzeugen. Die erste Übung behandelt den [C++17-Encoder](../assembler/01_encoder.md).
Die späteren Schritte ergänzen Textparser, Labels und ROM-Ausgabe.

Eine CPU v3 mit 8-Bit-Datenpfad und RAM ist ein späteres
Architekturprojekt, nicht Teil der CPU-v2-Integration.

---

[← Zurück: ROM16x8](rom16x8.md) · [Weiter: Mini-Assembler →](../assembler/01_encoder.md)
