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

## 5. Mögliche nächste Übungen

1. ISA und Verhalten der reservierten Opcode-Werte verbindlich machen.
2. `decoder8` mit neuer Schnittstelle und unabhängigen C++-Tests.
3. Neue ACC-/PC-Steuerlogik auf Basis der Befehle.
4. ROM 16×8 mit einem kleinen Programm, das mindestens einen Sprung benutzt.
5. Eigenständigen `cpu_v2_top` verbinden und gegen einen neuen Emulator testen.

Quellcode: [`src/cpu_v2/README.md`](../../src/cpu_v2/README.md).

Die Reihenfolge beschreibt Lernschritte, **keine bereits
implementierten Module oder bestandenen Tests**.
