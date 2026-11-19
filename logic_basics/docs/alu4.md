# Erste 4-Bit-ALU

## Ziel

Eine kombinatorische Arithmetic Logic Unit (ALU) aus bereits bekannten
Rechen- und Logikfunktionen aufbauen. Anders als Register oder FSM besitzt
die ALU in dieser Übung keinen Takt und keinen gespeicherten Zustand.

## Schnittstelle

`alu4_top` hat die vier Bit breiten Operanden `a` und `b`
sowie den zweibittigen Funktionsselektor `op`.

| op | Operation | Resultat (`result`) | `carry_out` |
|---:|---|---|---|
| 0 | ADD | `(a + b) mod 16` | Fünftes Bit der Summe |
| 1 | AND | `a & b` | false |
| 2 | OR | `a \| b` | false |
| 3 | XOR | `a ^ b` | false |

`zero` ist bei jeder Operation genau dann `true`, wenn
`result == 0`. Das Flag wird **nicht** aus einer fünf Bit breiten
Summe berechnet. Es ist daher möglich, dass bei ADD gleichzeitig
`zero = true` und `carry_out = true` gelten.

## Beispiel 1: Addition über die Registerbreite hinaus

```text
  1111 (15)
+ 0001 ( 1)
------------
1 0000 (16)
^ ^^^^
|    vier Bit Ergebnis = 0000
Carry = 1
```

Erwartet: `result = 0`, `carry_out = true`, `zero = true`.

## Beispiel 2: Logische Operationen

```text
a = 1010
b = 1100

AND = 1000
OR  = 1110
XOR = 0110
```

Bitweise Operatoren arbeiten parallel über alle vier Stellen.
Die logischen Operatoren `&&`, `||` und `^^` arbeiten dagegen mit `bool`.

## Wiederverwendung vorhandener Hardware

Im Modul `adders.spade` existiert bereits:

```spade
pub fn add4(a: uint<4>, b: uint<4>) -> uint<5>
```

In einer anderen Spade-Datei kann die Funktion über `use lib::adders;`
und `adders::add4(a, b)` aufgerufen werden.

Der fünfbittige Rückgabewert enthält sowohl den vierbittigen
Ergebnisanteil als auch das Carry-Bit. Die Aufteilung ist Teil der
Übung. In Spade kannst du beispielsweise `.as_bits()` nutzen.

## Hardwarebild

Alle benötigten Operationen lassen sich als parallele Rechenschaltungen
denken. Der Funktionsselektor entscheidet, welcher ihrer Ausgänge an
`result` erscheint. Ein `match op` eignet sich als Auswahlstruktur.

```text
a,b --> 4-Bit-Addierer --+
      --> AND ----------+
      --> OR -----------+--> Auswahl per op --> result[3:0]
      --> XOR ----------+

                ADD-CARRY --> Auswahl --> carry_out
                                         result==0 --> zero
```

## Testen

```sh
make test_alu4
```

Die C++-Testbench prüft systematisch alle vier Operationen mit allen
`16 * 16` Operandenpaaren, zusätzlich einige direkte Operationswechsel.
Insgesamt werden **1048 Fälle** geprüft. Eine VCD-Datei wird unter
`logic_basics/build/alu4.vcd` geschrieben.

Der VS-Code-Standard-Testtask führt nur den ALU-Test aus.
Die Implementierung ist inzwischen vorhanden. Die gemeldeten 1048 grünen
Tests stammen aus der lokalen Verilator-Simulation.

## Zwei sinnvolle Varianten für denselben Addierausgang

Die vorhandene Lösung mit `.as_bits()` ist korrekt und gut nachvollziehbar.
Sie beschreibt das Aufteilen von fünf bereits vorhandenen Signalleitungen.
Wenn kürzerer Quellcode gewünscht ist, bietet Spade die Methoden
`.trunc()` und `.msb()` an:

```spade
let sum = adders::add4(a, b);
let sum4: uint<4> = sum.trunc();
let carry = sum.msb();
```

Die Bitbreite von `sum4` ist explizit, damit `.trunc()` weiß, auf
wie viele Bits gekürzt werden soll. Eine weitere Möglichkeit wäre,
`a + b` direkt als fünf Bit breite Summe zu verwenden. Sie verzichtet
auf den selbst aufgebauten Ripple-Carry-Addierer, sagt aber noch nichts
über die Fläche oder die Laufzeit der daraus erzeugten Hardware aus.

## Erster Kontakt mit Yosys

Noch kein Variantenvergleich: Zunächst nur den bestehenden Entwurf messen.

```sh
make synth_alu4
```

Dieses Target baut erst das Spade-Projekt mit Swim und liest dann
`logic_basics/build/spade.sv` in Yosys ein. `synth -flatten -top alu4_top`
löst vor der Synthese alle Untermodulgrenzen auf, damit deren Logik in
der ausgegebenen Zellzahl enthalten ist. `stat` liefert die Zusammenfassung.

Das vollständige Protokoll liegt in `logic_basics/build/alu4_synth.log`.
Scrolle im Protokoll ans Ende zum Abschnitt `=== alu4_top ===`.
Dort stehen unter anderem die gezählten Zellen (`cells`).

Diese Anzahl ist **keine direkte FPGA-LUT-Zahl und keine gemessene
Taktfrequenz**. Sie hängt von Syntheseschritten und Zieltechnologie ab.
Ein späterer fairer Vergleich muss dieselbe Toolversion und dieselben
Optionen für beide Schaltungsvarianten benutzen. Zuerst interessiert
uns nur, wie eine Syntheseausgabe überhaupt aussieht.

## Experiment: selbst aufgebauter Addierer gegen `+`

Die originale ALU bleibt in `src/alu.spade` bestehen.
Die neue Datei `src/alu_native.spade` ist eine bewusst separate
Vergleichsvariante. Nur ein Ausdruck ändert sich:

```spade
// Original: let sum = adders::add4(a, b);
let sum: uint<5> = a + b;
```

Alle übrigen Operationen, die Ergebnisauswahl und die Zerlegung in
`result` und `carry_out` sind identisch.

### Schritt 1: Verhalten prüfen

```sh
make test_alu4_native
```

Die zusätzliche Testbench prüft dieselben 1048 Fälle wie der Originaltest.
Sie schreibt `logic_basics/build/alu4_native.vcd`.

### Schritt 2: Synthese vergleichen

```sh
make compare_alu4
```

Der Befehl ruft Yosys für jede Variante **in einem neuen Prozess** auf.
Beide Durchläufe verwenden dieselben drei Kommandos:

```text
read_verilog -sv build/spade.sv
synth -flatten -top <jeweiliges Top-Modul>
stat
```

Die Zusammenfassung zeigt die generischen Zellzahlen der **vollständigen** ALUs. Die vollständigen
Protokolle liegen in `build/alu4_ripple_compare.log` und
`build/alu4_native_compare.log`.

Das ist ein kontrollierter Vergleich der **Zellanzahl** unter gleichen
Synthesebedingungen, kein direkter Vergleich von Laufzeit, LUT-Belegung,
Transistorzahl oder Leistungsverbrauch. Später können wir dafür
konkrete Technologien und Timingberichte untersuchen.

### Achtung: 42 vs. 57 ohne `-flatten`

Der erste Variantenvergleich meldete 42 Zellen für die selbst gebaute
und 57 für die native Addition. Diese Zahlen stammen aus dem jeweiligen
Top-Modul und können hierarchische Unterbausteine auslassen. Der vorherige
Einzelbericht mit 62 Zellen passt zu diesem Verdacht; ohne die alten
Logdateien lässt sich der Unterschied aber nicht abschließend erklären.

Deshalb verwenden sowohl `synth_alu4` als auch `compare_alu4` jetzt
`synth -flatten`. Die damit gemessenen Zahlen sind eine neue,
vergleichbare Ausgangsbasis. Eine niedrigere Zellzahl bedeutet weiterhin
nicht automatisch kürzere Laufzeit oder weniger FPGA-LUTs.
