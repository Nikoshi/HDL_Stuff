# CPU v2, Kapitel VI: Steuerlogik fuer Maschinenbefehle (abgeschlossen)

**Status:** Abgeschlossen. Auch nach dem `fn`-/Testhüllen-Refactoring wurden **144/144 bestandene Vergleiche** gemeldet.

## 1. Warum brauchen wir Steuerlogik?

`decoder8_top` liefert lediglich `opcode` und `immediate`.
Ein Opcode ist aber noch kein Signal, das sich direkt an ein
Register anschliessen liesse.

Beispielsweise soll `LDI 5` den Akkumulator mit 5 laden,
`JMP 5` dagegen den Akkumulator **nicht** veraendern,
sondern den PC zur Adresse 5 fuehren.

Dafuer trennen wir zwei Aufgaben:

```text
instruction[7:0] -> decoder8 -> opcode[3:0]
                                   |
                                   v
                               control8
                           /      |       \
                  ACC-Freigabe  ALU-Wahl   PC-Modus
```

Die eigentliche Steuerentscheidung ist jetzt eine **reine Funktion**
`decode_control_signals`. Sie erzeugt kombinatorische Signale, keine
Register und keine Taktverzoegerung. Die Test-Entity
`test_control8_top` dient ausschliesslich der Verilator-Schnittstelle.
Die Daten `immediate` und `ACC` werden hier nicht ausgewertet.

## 2. Schnittstelle

Datei: `logic_basics/src/cpu_v2/control8.spade`

Die spaeter wiederverwendbare Funktion hat die Signatur:

```spade
pub fn decode_control_signals(opcode: uint<4>) -> (bool, bool, uint<2>, uint<2>)
```

Sie liefert in dieser Reihenfolge `acc_write`, `acc_from_imm`,
`alu_op`, `pc_mode`. Die separat exportierte **Testhülle**
`test_control8_top` stellt diese vier Rueckgabewerte als benannte
`inv`-Ports bereit. Darauf bezieht sich folgende Porttabelle:

| Port | Typ | Bedeutung |
|---|---|---|
| `opcode` | `uint<4>` | Oberes Nibble aus dem Felddecoder |
| `acc_write` | `inv bool` | ACC soll bei aktivem CPU-Schritt schreiben |
| `acc_from_imm` | `inv bool` | ACC-Datenquelle: Immediate statt ALU |
| `alu_op` | `inv uint<2>` | ALU-Funktion, kompatibel zur ALU aus CPU v1 |
| `pc_mode` | `inv uint<2>` | Art der naechsten PC-Adresse |

Der Wert `pc_mode` ist eine kleine Steuerkodierung:

| Wert | Bedeutung | Entscheidung in der spaeteren PC-Logik |
|---|---|---|
| `00` | SEQUENTIAL | PC + 1 (modulo 16) |
| `01` | JMP | PC := Immediate |
| `10` | JZ | Falls ACC_alt = 0, PC := Immediate, sonst PC + 1 |
| `11` | Reserviert | Wird aktuell von keinem gueltigen Opcode erzeugt |

**Wichtig:** `control8` entscheidet bei JZ nicht selbst, ob
gesprungen wird. Es erzeugt nur den Modus `10`. Erst die
spaetere PC-Logik bekommt auch den alten ACC-Wert und das Immediate.

## 3. Verbindliche Wahrheitstabelle

| Opcode | Befehl | `acc_write` | `acc_from_imm` | `alu_op` | `pc_mode` |
|---|---|---:|---:|---|---|
| `0x0` | ADD | 1 | 0 | `00` | `00` |
| `0x1` | AND | 1 | 0 | `01` | `00` |
| `0x2` | OR | 1 | 0 | `10` | `00` |
| `0x3` | XOR | 1 | 0 | `11` | `00` |
| `0x4` | LDI | 1 | 1 | `00` | `00` |
| `0x8` | JMP | 0 | 0 | `00` | `01` |
| `0x9` | JZ | 0 | 0 | `00` | `10` |
| `0xC` | NOP | 0 | 0 | `00` | `00` |
| `0x5..0x7`, `0xA..0xB`, `0xD..0xF` | Reserviert | 0 | 0 | `00` | `00` |

**Entscheidung fuer die Bastel-CPU:** Reservierte Opcodes haben
bis zur Definition neuer Befehle die **sicheren NOP-Steuersignale**.
Die Werte sind weiterhin *reserviert* und nicht als neue
Maschinenbefehle dokumentiert. Spaeter koennen wir diese
Steuerzuordnung mit einem neuen ISA-Befehl bewusst aendern.

Warum ist `alu_op=00` bei `LDI`, `JMP`, `JZ`
und `NOP` unproblematisch? Weil bei diesen Befehlen
entweder `acc_write=0` gilt oder `acc_from_imm=1` die
ALU-Ergebnisse als Datenquelle uebergeht. Die ALU darf
kombinatorisch trotzdem einen Wert berechnen.

## 4. Implementierung: Reine Funktion und Testhülle

Die Opcode-Wahrheitstabelle ist in `decode_control_signals` als
`match opcode` umgesetzt. Die Funktion liefert ein Tupel mit vier
Steuersignalen. Die `test_control8_top`-Entity entpackt diese Werte
in sprechend benannte Zwischenvariablen und leitet sie an ihre Ports
weiter. Sie fuegt keine weitere Logik und keinen Zustand hinzu.

Die exportierte `pub fn` kann spaeter von der CPU-v2-Steuerverdrahtung
verwendet werden; dazu muss nicht die Test-Entity instanziiert werden.

Die Signale `rst` und `advance` gehoeren **nicht** in
diese Entity: Der Top-Level kombiniert spaeter
`advance` mit `acc_write` und behandelt Reset mit
hoechster Prioritaet im jeweiligen Register.

### Gedankenbeispiel

```text
opcode = 0x9  (JZ)
  acc_write = 0
  acc_from_imm = 0
  alu_op = 00
  pc_mode = 10
```

Was passiert bei `ACC_alt=0` oder `ACC_alt=7`?
**Dieser Decoder gibt in beiden Faellen identische Werte aus.**
Die unterschiedliche PC-Wirkung entsteht erst spaeter.

## 5. Testen

Im Repository-Root:

```sh
make test_control8
```

Die unabhängige Testbench in `logic_basics/test/control8.cc`
prueft alle **16** Opcodes und danach **128** direkte,
nicht-monotone Signalwechsel, insgesamt **144 Vergleiche**.
Dabei sind auch alle acht reservierten Werte abgedeckt.

Der Null-/NOP-Platzhalter ist durch die fertige Funktion ersetzt.
Der Test wurde nach der Umstellung auf eine `fn` und
`test_control8_top` erneut ausgeführt: **144/144 Vergleiche bestanden**.
Die VCD-Datei liegt unter `logic_basics/build/control8.vcd`.

`make test_completed` umfasst inzwischen **22 abgeschlossene
Hardware-Testprogramme**. Der ROM16x8-Test gehört ebenfalls zur Regression.

## 6. Danach

Die [PC-/ACC-Zustandssteuerung](state_v2.md) ist ebenfalls abgeschlossen.
Als nächste eigenständige Übung folgt der [16×8-Bit-ROM](rom16x8.md).
Die CPU-v1-Logik bleibt unverändert.

Weiterfuehrend: [ISA-Entwurf](isa.md),
[Kapitel V: Felddecoder](decoder8.md).

---

[← Zurück: 8-Bit-Decoder](decoder8.md) · [Weiter: PC-/ACC-Zustand →](state_v2.md)
