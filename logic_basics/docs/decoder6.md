# 6-Bit-Befehlsdecoder: Theorie und Spade-Übung

## Ziel: Bits bekommen Bedeutung

Die ALU versteht bereits vier Operationen, der Akkumulator kann
Ergebnisse speichern. Der Decoder uebersetzt jetzt einen sechs Bit
breiten **Maschinenbefehl** in die beiden vorhandenen ALU-Eingaenge.
Er trifft noch keine Entscheidung ueber den naechsten Befehl
und kennt keinen Programmzaehler.

Unser bewusst kleiner Befehlssatz (ISA) verwendet ausschliesslich
Operationen mit einem unmittelbaren Wert (Immediate):

| op-Bits | Mnemonic | Berechnung im Akkumulator |
|---|---|---|
| 00 | ADD n | (ACC + n) modulo 16 |
| 01 | AND n | ACC & n |
| 10 | OR n | ACC \| n |
| 11 | XOR n | ACC ^ n |

Die ersten zwei Bits des Befehls waehlen die ALU-Operation aus,
die unteren vier Bits enthalten n (0 bis 15).
Das ergibt 2^6 = 64 moegliche Befehlswoerter.

## Verbindliches Bitlayout

```text
  Bitposition:   5   4   3   2   1   0
               +-------+---------------+
  instruction:  | op    | operand / n   |
               +-------+---------------+
                   2            4   Bits
```

Beispiel: `10_1101` bedeutet `OR 13`:
- `10` => op = 2 => OR
- `1101` => operand = 13

Das Format ist von uns fuer die Lern-CPU definiert, kein Standard-ISA.
Die Hardware orientiert sich an diesem Format, nicht umgekehrt.

## Decoder ist keine Recheneinheit

Ein Decoder **fuehrt die Operation nicht aus**. Er trennt die
Eingangssignale nur nach vereinbarter Bitposition.
Die ALU bekommt danach `op` und `operand`; das Register bekommt
später das Ergebnis.

In dieser einfachen Form braucht der Decoder keine Zustandsmaschine,
keine Speicherzellen und normalerweise nur direkte Leitungsverbindungen.
Wenn sich der Befehl aendert, aendern sich die Ausgaenge kombinatorisch.

Eine Besonderheit: Wir koennen *alle* 64 Eingaben testen, statt
nur zufaellige Beispiele zu waehlen.

## Spade: Was du fuer die Uebung brauchst

Der fertig implementierte Decoder liegt unter
`src/cpu/decoder6.spade` und verdrahtet die beiden Bitfelder so:

```spade
let bits = instruction.as_bits();
set op = [bits[4], bits[5]].as_uint();
set operand = [bits[0], bits[1], bits[2], bits[3]].as_uint();
```

Die Entity selbst wurde als `pub entity` für die CPU-Integration exportiert.

### 1. Die sechs Bits sichtbar machen

Ein `uint<6>` ist eine einzelne sechs Bit breite Zahl.
`.as_bits()` liefert dazu sechs `bool`-Werte:

```spade
let bits = instruction.as_bits();
let lowest = bits[0];  // Bit 0, LSB
let highest = bits[5]; // Bit 5, MSB
```

**Wichtig:** Die Bitfolge im Array geht von niedriger zu
hoeherer Wertigkeit. Sie ist nicht die gewohnte Leserichtung
einer dargestellten Binaerzahl.

### 2. Bits in neue Zahlen umwandeln

Das Muster kennen wir aus der ALU: Ein Array aus Bool-Bits
laesst sich mit `.as_uint()` als unsigned Integer interpretieren.
Ein **isoliertes** Beispiel:

```spade
let data: uint<3> = [bits[0], bits[1], bits[2]].as_uint();
```

Hier wird aus den drei niederwertigsten Bits ein dreibittiger
Wert. Fuer die Uebung brauchst du zwei aehnliche Ausdruecke
mit unterschiedlichen Bitbereichen und Zielbreiten.

### 3. Die Entity-Ausgaenge beschalten

Die `inv`-Portparameter `op` und `operand` sind Rueckwaertssignale,
die in der Entity mit `set` verbunden werden:

```spade
set op = ...;
set operand = ...;
```

Es gibt hier bewusst kein `clk`, kein `reg`, kein `inst`
und keinen `port()`-Aufruf: Der Decoder ist in dieser Uebung
der selbststaendige Test-Top. Beim spaeteren CPU-Zusammenbau
werden wir ihn wiederverwenden.

### 4. Ein zweiter Loesungsweg zum Nachdenken

Statt einzelne Bits auszulesen kann man ein Bitfeld konzeptionell
auch durch Schieben und Maskieren isolieren. In unserem
C++-Referenzmodell passiert genau das:

```cpp
op = (instruction >> 4) & 0x3;
operand = instruction & 0xF;
```

Das ist **Software-Syntax**, kein ungepruefter Spade-Schnipsel.
Fuer die erste Umsetzung bleiben wir bewusst bei
`.as_bits()` und `.as_uint()`, die du bereits benutzt hast.

## Testbenches und Emulatoridee

Mit `make test_decoder6` pruefen wir 64 vollstaendige
Befehlscodes und 32 unmittelbare Eingangswechsel, also
96 Pruefungen. Die Ergebnisse werden in
`logic_basics/build/decoder6.vcd` aufgezeichnet.

Das unabhängige Software-Referenzmodell der Befehlsausführung liegt
inzwischen in `test/instruction6_model.hpp`. Wenn wir spaeter auch den
Programmzaehler und einen Programmspeicher haben, kann
ein kleiner Emulator dieselben Befehle nacheinander
ausfuehren und seinen Zustand (`PC`, `ACC`, gegebenenfalls
Flags) mit dem Spade-Design vergleichen.

**Unterschied:** Verilator simuliert unsere tatsaechliche
Hardwarebeschreibung. Ein Emulator implementiert dagegen
die *vorgeschriebene Wirkung* eines Befehls direkt als Software.
Wenn beide bei jeder ausgefuehrten Instruktion denselben Zustand
liefern, ist das ein starker, unabhaengiger Funktionstest.

## Stand und nächster Schritt

Die ursprüngliche Übung ist mit **96/96 Tests** bestanden.
Der Decoder ist nun eine öffentliche Entity, die in
`instruction_exec6.spade` wiederverwendet werden kann.
Im nächsten Schritt verbinden wir ihn mit dem Akkumulator,
ohne die Bitlogik erneut zu schreiben.
