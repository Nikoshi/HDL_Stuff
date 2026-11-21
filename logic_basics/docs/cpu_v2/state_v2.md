# CPU v2, Kapitel VII: PC- und ACC-Zustandssteuerung

**Status:** Abgeschlossen. Für die implementierte Registersteuerung wurden
**69.635/69.635 Zustandsvergleiche** im lokalen Verilator-Test gemeldet.

## 1. Was dieser Baustein macht

Die CPU v2 speichert zwei unabhängige 4-Bit-Werte:

- **ACC:** Das Datenregister für Rechenergebnisse und geladene Konstanten.
- **PC:** Die Adresse, von der der nächste Befehl gelesen wird.

`decoder8` zerlegt ein Befehlswort; `decode_control_signals` legt dessen
Steuersignale fest. `cpu_v2_state_top` führt diese Befehle **nicht selbst
aus**: Er bekommt die Steuersignale und ein bereits berechnetes
`alu_result` als Eingänge und entscheidet nur, **was beim nächsten
Takt in PC und ACC gespeichert wird**.

```text
                                 ┌─────────────────┐
 immediate ──────────────────────┤                 │
 alu_result ─────────────────────┤ ACC-Datenwahl   ├──> ACC-Register
 acc_write, acc_from_imm ────────┤                 │         │
                                 └─────────────────┘         │
                                                             │ alter ACC
                                                             v
 pc_mode, immediate ─────────────┐                     Nullprüfung (JZ)
                                 v                           │
 aktueller PC ───────────> PC-Folgewertwahl <──────────────────┘
                                 │
                                 v
                            PC-Register
```

**Gedankenmodell:** Vor einer Taktflanke stehen `PC_alt` und `ACC_alt`
fest. Die kombinatorische Logik berechnet `PC_neu` und `ACC_neu`
daraus. Erst mit der steigenden Flanke werden beide neuen Werte
**gleichzeitig** gespeichert.

## 2. Schnittstelle

Datei: `logic_basics/src/cpu_v2/state_v2.spade`

Top-Level: `cpu_v2_state_top`

| Port | Spade-Typ | Aufgabe |
|---|---|---|
| `clk` | `clock` | Steigende Flanke übernimmt Folgezustände |
| `rst` | `bool` | Beide Register synchron auf null setzen |
| `advance` | `bool` | Gesamte CPU für einen Schritt freigeben |
| `immediate` | `uint<4>` | Konstante oder absolute Sprungadresse |
| `alu_result` | `uint<4>` | Berechnetes Ergebnis der vorgelagerten ALU |
| `acc_write` | `bool` | Schreiben des ACC erlauben |
| `acc_from_imm` | `bool` | Immediate statt ALU-Ergebnis auswählen |
| `pc_mode` | `uint<2>` | Normal, JMP, JZ oder Fallback |
| `pc` | `inv uint<4>` | Ausgang: aktuell gespeicherter PC |
| `acc` | `inv uint<4>` | Ausgang: aktuell gespeicherter ACC |

Die vier Steuersignale (`acc_write`, `acc_from_imm`, `pc_mode`
und die andernorts genutzte ALU-Operation) stammen später aus
`decode_control_signals`. Im Einzeltest werden sie direkt eingespeist.
So ist die Registersteuerung unabhängig vom Opcode-Decoder prüfbar.

## 3. Zuerst die zwei Speicherentscheidungen

### Entscheidung A: Was passiert mit dem ACC?

Der erste Schritt ist **Schreibfreigabe**, der zweite **Datenquelle**:

| `advance` | `acc_write` | `acc_from_imm` | Nächster ACC |
|---:|---:|---:|---|
| 0 | beliebig | beliebig | `ACC_alt` halten |
| 1 | 0 | beliebig | `ACC_alt` halten |
| 1 | 1 | 0 | `alu_result` |
| 1 | 1 | 1 | `immediate` |

Ein `LDI` verwendet die letzte Zeile. Bei `JMP` und `JZ`
bleibt der ACC erhalten, weil `acc_write=0` ist.

### Entscheidung B: Was passiert mit dem PC?

Bei `advance=0` bleibt der PC unverändert. Andernfalls gilt:

| `pc_mode` | Bedeutung | Nächster PC |
|---|---|---|
| `00` | Normal | `(PC_alt + 1) mod 16` |
| `01` | JMP | `immediate` |
| `10` | JZ, wenn `ACC_alt = 0` | `immediate` |
| `10` | JZ, wenn `ACC_alt != 0` | `(PC_alt + 1) mod 16` |
| `11` | Reserviert | `(PC_alt + 1) mod 16` |

Der vierte Modus ist **kein ISA-Befehl**, sondern ein definierter
sicherer Fallback bei einer ansonsten unbenutzten Steuerkodierung.
Das 4-Bit-Inkrement erfolgt in Spade über `pc_reg +. 1`
und läuft von 15 auf 0 zurück.

## 4. Reset, Pause und Taktflanke

Die Priorität ist für **beide Register gleich**:

1. **Synchroner Reset:** An einer steigenden Taktflanke mit `rst=1` werden PC und ACC null, auch wenn `advance=0` ist.
2. **Pause:** Bei `rst=0` und `advance=0` bleiben beide Register unverändert.
3. **Freigegebener Schritt:** Bei `rst=0` und `advance=1` gelten die Auswahlentscheidungen aus Abschnitt 3.

Eine bloße Änderung der Eingänge, einschließlich `rst`, aktualisiert
noch kein Register. Auch die fallende Taktflanke speichert nichts.
`reg(clk)` mit `reset(rst: 0)` realisiert den synchronen Reset.

## 5. Weshalb JZ den gespeicherten ACC liest

Die JZ-Bedingung bezieht sich auf `ACC_alt`, **nicht** auf
`alu_result`. Die ALU kann einen ganz anderen Wert berechnen.

Ein Beispiel aus dem Vertrag des Zustandsblocks:

| Vor der Flanke | Wert |
|---|---:|
| `PC_alt` | 6 |
| `ACC_alt` | 0 |
| `immediate` | 12 |
| `alu_result` | 9 |
| `pc_mode` | `10` |
| `advance`, `acc_write` | je 1 |
| `acc_from_imm` | 0 |

An derselben steigenden Flanke wird **PC=12** (JZ sieht `ACC_alt=0`)
und **ACC=9** (ALU-Ergebnis) gespeichert. Es gibt kein zeitliches
„erst ACC, dann PC“: Beide lesen den bisherigen Zustand.

Der aktuelle Opcode-Decoder erzeugt die Kombination
„JZ und ACC schreiben“ nicht. Der Zustandsblock wird trotzdem
auch mit solchen direkten Steuersignalen getestet, um seine
allgemeinen Zustandsregeln unabhängig abzusichern.

## 6. Spade-Umsetzung: Eine Entity und zwei Register

Im Gegensatz zum kombinatorischen `control8` besitzt dieser
Baustein Zustand und benötigt deshalb eine `entity` mit
zwei `reg(clk)`-Registern.

Wichtig ist die Unterscheidung zwischen
**Quellcodereihenfolge und Hardwarezeit**:

- Der PC-Ausdruck steht im Code vor dem ACC-Register.
- Für `JZ` muss dieser Ausdruck trotzdem den gespeicherten `acc_reg` lesen.
- `decl acc_reg;` macht den später definierten Registerwert bereits
  vorher im Quellcode sichtbar. Es erzeugt **kein drittes Register**.

Die Stelle in der Implementierung:

```spade
decl acc_reg;

// Die JZ-Bedingung innerhalb der PC-Berechnung liest acc_reg.
// Weiter unten folgt die tatsächliche Registerdefinition.
```

Die beiden Namen `pc_reg` und `acc_reg` bezeichnen die gespeicherten
Werte. Zusätzliche `next_*`-Signale sind möglich, aber für diese
kleine Schaltung nicht zwingend.

## 7. Verifikation

Im Repository-Root:

```sh
make test_state_v2
```

Die unabhängige Testbench `logic_basics/test/state_v2.cc`
berechnet `PC_neu` und `ACC_neu` aus dem alten Referenzzustand.
Sie prüft:

- Alle 256 Kombinationen aus `PC_alt` und `ACC_alt`.
- Alle vier PC-Modi, beide ACC-Steuerbits und wechselnde Datenwerte.
- Reset und Pause, auch bei gegensätzlichen Steuersignalen.
- JZ mit ACC gleich und ungleich null.
- Eingangsänderungen bei hohem Takt und fallende Flanken.
- Eine längere gemischte Zustandsfolge.

Insgesamt ergeben sich **69.635 Zustandsvergleiche**. Der erste
Eingangswechsel vor der initialen Reset-Flanke wird nicht gewertet,
da die Register bis dahin noch keinen definierten Zustand haben.
Das Signalprotokoll wird unter
`logic_basics/build/state_v2.vcd` gespeichert.

Die Registersteuerung ist damit als isolierter Baustein überprüft.
Es ist **noch kein Test einer vollständig verschalteten CPU v2**.

## 8. Weiter zum Programmspeicher

Der nächste Baustein wird ein **16×8-Bit-ROM** mit dem
[Countdown-/LED-Alarm-Programm](isa.md). Erst anschließend können
Programmspeicher, Decoder, Steuerfunktion, ALU und Zustandsregister
zur CPU v2 verbunden werden.

Vorherige Kapitel: [Felddecoder](decoder8.md) und
[Steuerdecoder](control8.md).
