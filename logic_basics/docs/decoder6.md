# 6-Bit-Befehlsdecoder: Theorie und Spade-Uebung

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

Der bereits vorhandene Top-Level ist:

```spade
#[no_mangle(all)]
entity decoder6_top(
    instruction: uint<6>,
    op: inv uint<2>,
    operand: inv uint<4>
) {
    // TODO: beide Ausgaenge passend beschalten
    set op = 0u2;
    set operand = 0u4;
}
```

Die beiden Nullwerte sind **nur Platzhalter**. Wie beim
Akkumulator soll der Test bis zur Implementierung scheitern.

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

Der C++-Test enthaelt bereits die ersten Zeilen eines
Software-Referenzmodells. Wenn wir spaeter auch den
Programmzaehler und einen Programmspeicher haben, kann
ein kleiner Emulator dieselben Befehle nacheinander
ausfuehren und seinen Zustand (`PC`, `ACC`, gegebenenfalls
Flags) mit dem Spade-Design vergleichen.

**Unterschied:** Verilator simuliert unsere tatsaechliche
Hardwarebeschreibung. Ein Emulator implementiert dagegen
die *vorgeschriebene Wirkung* eines Befehls direkt als Software.
Wenn beide bei jeder ausgefuehrten Instruktion denselben Zustand
liefern, ist das ein starker, unabhaengiger Funktionstest.

## Arbeitsauftrag

1. Ersetze die beiden Platzhalter in `src/decoder6.spade`.
2. Pruefe die Feldzuordnung mit `10_1101` (OR 13).
3. Fuehre `make test_decoder6` aus.
4. Stelle sicher, dass die Ausgaenge ohne Takt sofort folgen.

Nach bestandenem Decoder testen wir seine Integration in den
Akkumulator. Dann bekommen unsere sechs Bit erstmals eine
**ausfuehrbare** Bedeutung.
