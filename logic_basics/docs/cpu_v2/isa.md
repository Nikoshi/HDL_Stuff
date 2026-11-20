# CPU v2: Entwurf der 8-Bit-Befehlssatzarchitektur

**Status:** ISA-Entwurf vor der Hardware-Implementierung.
Die erste 4-Bit-CPU in `src/cpu/` bleibt der unveränderte,
vollständig getestete Referenzstand.

## 1. Warum eine zweite Version?

Die CPU v1 verwendet sechs Befehlsbits: zwei für den Opcode und
vier für den unmittelbaren Operanden. Dadurch sind ihre vier Opcodes
bereits vollständig belegt.

CPU v2 behält einen **4-Bit-Datenpfad** (ACC und PC), verwendet aber
**8-Bit-Befehle**. Wir planen die Steuerlogik, bevor Decoder, ROM und
Ausführungsstufe verändert werden.

```text
Bitposition:    7 6 5 4 | 3 2 1 0
                O O O O | I I I I
                Opcode  | Immediate
                  4 Bit | 4 Bit
```

`instruction[7:4]` bezeichnet die vier Opcode-Bits,
`instruction[3:0]` die vier Immediate-Bits (Bitpositionen
in aufsteigender Wertigkeit ab 0 gezählt). Der Operand ist
je nach Befehl entweder eine 4-Bit-Konstante oder eine
absolute 4-Bit-Programmadresse.

## 2. Vorläufige Opcode-Tabelle

Der Opcode wird zur leichteren Orientierung in Vierergruppen
geordnet. Diese Gruppierung ist **keine Pflicht**, sie direkt
als eigene Hardwarehierarchie nachzubauen.

| Opcode | Name | Effekt auf ACC | Effekt auf PC |
|---|---|---|---|
| `0000` | ADD | `ACC + immediate` (mod 16) | `PC + 1` (mod 16) |
| `0001` | AND | `ACC & immediate` | `PC + 1` |
| `0010` | OR | `ACC \| immediate` | `PC + 1` |
| `0011` | XOR | `ACC ^ immediate` | `PC + 1` |
| `0100` | LDI | `ACC = immediate` | `PC + 1` |
| `0101–0111` | reserviert | Noch nicht festgelegt | Noch nicht festgelegt |
| `1000` | JMP | ACC halten | `PC = immediate` |
| `1001` | JZ | ACC halten | `ACC == 0 ? immediate : PC + 1` |
| `1010–1011` | reserviert | Noch nicht festgelegt | Noch nicht festgelegt |
| `1100` | NOP | ACC halten | `PC + 1` |
| `1101–1111` | reserviert | Noch nicht festgelegt | Noch nicht festgelegt |

Damit sind zunächst **acht Befehle** definiert; acht weitere
Codierungen bleiben für spätere Aufgaben frei.

**Wichtig:** Reservierte Werte haben noch keine festgelegte
Ausführungssemantik. Die ersten CPU-v2-Programme und Tests verwenden
nur definierte Opcodes. Vor einer vollständigen Decoder-Testbench
legen wir das Verhalten ungültiger Opcodes ausdrücklich fest.

## 3. Was an einer Taktflanke geschehen soll

Vorerst bleibt die CPU eine einfache, nicht-pipelined Architektur:
Der kombinatorische ROM liefert `ROM[PC_alt]`, und die Register
übernehmen an der steigenden Taktflanke gleichzeitig die aus dem
**alten** PC- und ACC-Zustand berechneten Folgezustände.

`rst` soll synchron Vorrang haben. `advance = 0` soll beide
Register halten. Bei `advance = 1` bestimmt der jeweilige
Maschinenbefehl den nächsten Zustand.

Die neue Besonderheit: Anders als bei v1 soll `JMP` beziehungsweise
`JZ` die PC-Entscheidung **aus dem gelesenen Maschinenbefehl**
ableiten. Das bisherige externe `jump`/`target`-Interface wird
nicht stillschweigend zur v2-Spezifikation übernommen.

Für `JZ` prüfen wir den **bisherigen ACC-Wert**, nicht ein
vorläufiges ALU-Ergebnis derselben Flanke.

## 4. Geplanter Steuerdatenpfad

```text
             PC ──> ROM 16x8 ──> Opcode + Immediate
              ^                         |
              |                         v
              +──── PC-Steuerung <── Steuerdecoder
                                          |
                      +-------------------+
                      v
             ACC ──> ALU / Immediate-Auswahl ──> ACC
```

Ein einfacher `match opcode` kann zunächst bestimmen:

- Ob der Akkumulator geschrieben wird.
- Ob er das ALU-Ergebnis oder das Immediate übernimmt.
- Ob der PC inkrementiert oder zu einer Immediate-Adresse springt.
- Ob eine Bedingung wie `ACC == 0` erfüllt ist.

Die bestehende 4-Bit-ALU beherrscht die vier benötigten
Rechenoperationen bereits. Sie benötigt daher **nicht**
automatisch einen eigenen 4-Bit-Operationscode.

Eine nützliche Frage für die nächste Übung: Welche Steuersignale
können wir so wählen, dass `LDI`, `JMP`, `JZ` und `NOP`
keine künstlichen ALU-Operationen werden?

## 5. ISA-Praxistest: Countdown mit LED-Alarm

Bevor wir einen Decoder implementieren, prüfen wir die ISA anhand
eines Programms, das Schleifen, eine Bedingung und unterschiedliche
Arten von Zustandsänderungen kombiniert.

**Vereinbarung:** Die vier ACC-Bits stellen in dieser Simulation vier
*virtuelle LEDs* dar. Es gibt noch keinen echten Ausgabeport; jede
ACC-Änderung wird unmittelbar als LED-Muster sichtbar.

| PC | Hex-Wort | Befehl | Zweck |
|---:|---|---|---|
| 0 | `0x45` | `LDI 5` | Countdown initialisieren |
| 1 | `0x0F` | `ADD 15` | Eins abziehen (4-Bit-Modulo) |
| 2 | `0x95` | `JZ 5` | Bei ACC=0 Alarm starten |
| 3 | `0x81` | `JMP 1` | Sonst weiter herunterzaehlen |
| 4 | `0xC0` | `NOP` | In diesem Ablauf nicht erreichbar |
| 5 | `0x2F` | `OR 15` | LEDs: `1111` |
| 6 | `0x1A` | `AND 10` | LEDs: `1010` |
| 7 | `0x3F` | `XOR 15` | LEDs: `0101` |
| 8 | `0xC0` | `NOP` | Muster einen Takt halten |
| 9 | `0x80` | `JMP 0` | Countdown neu beginnen |
| 10–15 | `0xC0` | `NOP` | Unbenutzte ROM-Adressen |

Beispiel: `0x95 = 1001_0101` enthält den Opcode `1001` für
`JZ` und das Sprungziel `0101`, also Adresse 5.

Der erste Durchlauf benötigt **20 steigende Taktflanken**:
`LDI` einmal, fünf Subtraktionen mit `ADD 15`, fünf
`JZ`-Prüfungen, vier Rücksprünge in der Countdownschleife
und die fünf Alarmbefehle an den Adressen 5–9.
Nach dem letzten `JMP 0` steht der PC wieder auf 0;
der ACC bleibt bis zum nächsten `LDI` auf 5.

Die 20-Schritt-Zählung meint die ausgeführten Befehle
einschließlich der Sprünge, nicht 20 unterschiedliche ROM-Adressen.

### Unabhängiger Software-Emulator

Im Repository-Root:

```sh
make demo_cpu_v2
```

Das Target kompiliert nur `logic_basics/test/cpu_v2_isa_demo.cc`
mit dem C++-Compiler. Es braucht weder `swim build` noch
Verilator und erstellt **keine** VCD-Datei.

Der Emulator zeigt je Schritt den **alten PC**, das tatsächlich
ausgeführte Maschinenwort, den **alten und neuen ACC**, den
**neuen PC** und die vier LED-Bits an. Zusätzlich prüft er
Codierung, beide JZ-Zweige, LED-Muster, NOP, Rücksprung,
Pause und Reset anhand konkreter Erwartungen.

**Ergebnis des ersten ISA-Tests:** Alle acht vorgesehenen
Befehle reichen für dieses Programm aus. Das ist noch
kein Beweis, dass die ISA für jedes spätere Programm genügt.

**Offene Designfrage:** Der ACC dient gleichzeitig als Zähler
und als LED-Ausgabe. Ein späteres `OUT`-Kommando oder ein
separates Ausgaberegister könnte beide Aufgaben trennen.
Für dieses erste Demo ist die direkte ACC-Beobachtung beabsichtigt.

Die Ausführung reservierter Opcodes bleibt absichtlich
unspezifiziert. Der Demo-Emulator meldet sie als Fehler, damit
sich nicht versehentlich neue Befehlssemantik einschleicht.

## 6. Mögliche nächste Übungen

1. **Aktuelle Aufgabe:** [`decoder8` als rein kombinatorischen Felddecoder implementieren](decoder8.md), mit 384 unabhängigen Vergleichen.
2. Vor dem `control8`-Test die Behandlung reservierter Opcode-Werte festlegen.
3. ACC-/PC-Steuerlogik aus der neuen Steuersignaltabelle ableiten.
4. ROM 16×8 mit dem Countdown- und LED-Alarm-Programm bauen.
5. Eigenständigen `cpu_v2_top` verbinden und gegen einen neuen Emulator testen.

Quellcode: [`src/cpu_v2/README.md`](../../src/cpu_v2/README.md).

Die Reihenfolge beschreibt Lernschritte, **keine bereits
implementierten Module oder bestandenen Tests**.
