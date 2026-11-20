# CPU-Datenpfad II: Maschinenbefehle ausführen

## Idee

Bisher wurden der 6-Bit-Befehlsdecoder und der 4-Bit-Akkumulator
getrennt getestet. Jetzt werden diese **zwei existierenden Entities**
zusammengeschaltet. Die neue Einheit hat nur noch einen Eingang
für den gesamten Maschinenbefehl statt getrennter Eingänge für
`op` und `b`.

```text
instruction[5:0]
       |
       v
 +--------------+
 | decoder6_top |
 +--------------+
    |        |
 op[1:0]  operand[3:0]
    |        |
    v        v
 +------------------+       clk / rst / enable
 | accumulator4_top | <--------------------+
 +------------------+
         |
         v
       q[3:0]
```

Es ist kein zusätzlicher Akkumulator und kein zweiter ALU-Code
erforderlich. Der Decoder selbst bleibt kombinatorisch.
Nur der bereits vorhandene Akkumulator speichert einen Zustand.

## Exaktes Verhalten

`instruction_exec6_top` hat diese Ports:

| Port | Typ | Bedeutung |
|---|---|---|
| clk | clock | Steigende Flanke übernimmt einen Befehl |
| rst | bool | Synchroner Reset des gespeicherten ACC |
| enable | bool | Ausführung freigeben |
| instruction | uint<6> | Maschinenbefehl |
| q | inv uint<4> | Gespeicherter Akkumulatorwert |

An einer steigenden Flanke gilt:

- `rst=1`: ACC wird auf 0 gesetzt, unabhängig von `enable`.
- `rst=0` und `enable=1`: Genau ein aktueller Befehl wird ausgeführt.
- `rst=0` und `enable=0`: ACC bleibt unverändert.

Der Befehl wird nicht intern gespeichert. Die Kombination aus Opcode
und Immediate muss vor der steigenden Flanke gültig anliegen.
Ein Wechsel bei hohem Takt ohne neue steigende Flanke verändert ACC nicht.

### Beispielprogramm von Hand

| Schritt | Befehl (6 Bit) | Bedeutung | ACC danach |
|---|---|---|---:|
| Reset | - | ACC := 0 | 0 |
| 1 | `10_0101` | OR 5 | 5 |
| 2 | `00_1011` | ADD 11 | 0 |
| 3 | `11_0111` | XOR 7 | 7 |
| 4 | `01_0011` | AND 3 | 3 |

Beim zweiten Schritt tritt ein 4-Bit-Ueberlauf auf: 5 + 11 = 16,
im Akkumulator bleibt nur das untere Ergebnis `0000`.

## Spade: Hier zählt die Verdrahtung

Beide Bausteine sind im Repository jetzt als `pub entity` deklariert.
Du kannst sie aus dem neuen Modul importieren:

```spade
use lib::cpu::decoder6;
use lib::cpu::accumulator;
```

Die aktuelle Spade-Version im Projekt erwartet bei `inst`
**Positionsargumente**. Benannte Argumente haben bei der
Akkumulator-Uebung einen Compilerfehler erzeugt.
Die relevante Reihenfolge steht in den Entity-Schnittstellen:

```text
decoder6_top(instruction, op, operand)
accumulator4_top(clk, rst, enable, b, op, q)
```

Die `inv`-Ausgänge müssen wieder mithilfe von `port()`
mit lesbaren Signalen verbunden werden. Das kennst du
aus `accumulator.spade`.

**Drei Signalpaare** sind nötig: Opcode, Operand und akkumuliertes
Ergebnis. Das Resultat des Akkumulators kannst du anschließend
über `set q = ...` nach außen geben.

Die Funktionen von Decoder und Akkumulator nicht kopieren:
Der Lerngewinn liegt in *Hierarchie und Schnittstellen*.
Im Gegensatz zur vorigen Aufgabe ist dafür keine neue
`reg`-Anweisung notwendig.

## Alternative Architektur zum Vergleichen

Man könnte die sechs Instruktionsbits direkt im Top-Modul
aufteilen und `accumulator4_top` ohne separate Decoder-Entity
aufrufen. Das wäre für sechs Bits kürzer, versteckt aber
die im ISA-Vertrag definierte Übersetzung.

Für unsere erste CPU ist die getrennte Decoder-Entity deshalb
absichtlich besser: Bei zukünftigen Befehlstypen kann die
Steuerlogik wachsen, ohne dass der Akkumulator sie kennen muss.

## Das erste Stück CPU-Emulator

Im neuen C++-Header `test/instruction6_model.hpp` liegt ein
softwarebasiertes Referenzmodell. Es hält `acc` als normale
C++-Variable und interpretiert jede Instruktion direkt
mit Schieben, Maskieren und einer `switch`-Anweisung.

Beim Test werden die Eingaben gleichzeitig an die von Verilator
simulierte Spade-Schaltung und an den Emulator gegeben.
Nach jeder steigenden Flanke vergleichen wir deren ACC-Werte.

Der Emulator bildet **noch keine komplette CPU** ab:
`PC`, Programmspeicher und selbstständiges Befehls-Fetching
folgen in späteren Schritten. Hier modelliert er lediglich
die Ausführungsstufe unabhängig von der Schaltungsstruktur.

## Prüfumfang und Vorgehen

`make test_instruction_exec6` prüft:

1. Alle 64 Befehle mit jedem der 16 möglichen ACC-Vorwerte.
2. Änderungen der Instruktion bei bereits hohem Takt.
3. Enable-Pausen, Reset-Priorität und mehrere Befehle hintereinander.

Das ergibt **16.902 Ausgabevergleiche**. Der Test schreibt
`logic_basics/build/instruction_exec6.vcd`.

Der Platzhalter in `src/cpu/instruction_exec6.spade` setzt den
Ausgang noch auf 0. Nach dem Implementieren zuerst
`make test_instruction_exec6` ausführen, dann `make test`
für die Regression.

## Beobachtung für die spätere CPU

Die Ausführungsstufe benötigt noch kein Befehlsregister.
Wenn später ein Programmspeicher den Befehl liefert,
müssen wir entscheiden, ob der Befehl direkt kombinatorisch
aus dem Speicher kommt oder erst in einem Register landet.
Damit lernen wir als Nächstes, warum eine CPU häufig
einen geregelten Fetch-/Execute-Ablauf hat.
