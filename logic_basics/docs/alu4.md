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
Erst nach der Umsetzung sollte der Test grün sein.
