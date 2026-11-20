# CPU v2, Kapitel VI: Steuerlogik fuer Maschinenbefehle

**Status:** Offene TDD-Uebung. Der 8-Bit-Felddecoder ist abgeschlossen.

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

`control8_top` bleibt **kombinatorisch**: keine Register,
kein `clk`, kein `rst`, keine 8-Bit-Instruktion als Eingang.
Die Daten `immediate` und `ACC` selbst werden hier noch nicht
veraendert oder ausgewertet.

## 2. Schnittstelle

Datei: `logic_basics/src/cpu_v2/control8.spade`

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

## 4. Uebung: Opcode in Steuersignale uebersetzen

In `control8.spade` findest du einen bereits kompilierbaren
Platzhalter. Er gibt NOP-Werte fuer alle Eingaben aus.
Deine Aufgabe ist, die Wahrheitstabelle in kombinatorischer
Spade-Logik umzusetzen, **ohne Register**.

Du kannst dich an `match` aus der bisherigen ALU orientieren.
Du musst aber weder die ALU instanziieren noch eine neue
Registersteuerung schreiben.

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

Der NOP-Platzhalter soll zunaechst **fehlschlagen**.
Die VCD-Datei liegt nach dem Simulationslauf unter
`logic_basics/build/control8.vcd`.

`make test_completed` enthaelt die abgeschlossenen
19 Hardware-Tests; `make test` fuehrt auch die offene
Steuerdecoder-Uebung aus.

## 6. Danach

Erst wenn die Steuersignale korrekt sind, entwerfen wir
die eigentlichen Zustandsuebergaenge von PC und ACC
fuer `LDI`, `JMP` und `JZ`. Die bereits bewiesene
CPU-v1-Logik bleibt dabei unveraendert.

Weiterfuehrend: [ISA-Entwurf](isa.md),
[Kapitel V: Felddecoder](decoder8.md).
