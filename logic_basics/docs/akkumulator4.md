# Erste CPU-Baugruppe: 4-Bit-Akkumulator

## Lernziel

Eine bereits vorhandene kombinatorische ALU mit einem synchronen Register
verbinden, ohne die ALU-Operationen ein zweites Mal zu implementieren.
Der Akkumulator bildet die erste gespeicherte Rechenschleife unserer CPU.

## Verhalten

Die Schnittstelle `accumulator4_top` enthält
`clk: clock`, `rst: bool`, `enable: bool`, `b: uint<4>`,
`op: uint<2>` und den Ausgang `q: inv uint<4>`.

`q` zeigt den **aktuell gespeicherten** 4-Bit-Wert. Die ALU
verwendet diesen Wert als ersten Operanden `a`, während `b`
extern zugeführt wird.

Die Operationen entsprechen exakt unserer ALU:

| op | Operation | Aktualisierung bei enable |
|---:|---|---|
| 0 | ADD | `q + b`, modulo 16 |
| 1 | AND | `q & b` |
| 2 | OR | `q \| b` |
| 3 | XOR | `q ^ b` |

Zustandsregel an **steigenden Taktflanken**:

1. Wenn `rst = true`: `ACC := 0` (höchste Priorität).
2. Sonst bei `enable = true`: `ACC := ALU(ACC, b, op).result`.
3. Sonst: `ACC` bleibt unverändert.

Ändern sich `b`, `op`, `rst` oder `enable` ohne steigende
Taktflanke, darf `q` **nicht** sofort reagieren. Eine fallende
Flanke schreibt ebenfalls nichts ins Register.

## Warum keine kombinatorische Schleife entsteht

```text
      +-------------------+
 b -->|        ALU        |--> ALU-Ergebnis
 op ->|    a = ACC        |         |
      +-------------------+         v
               ^            +---------------+
               |            | Register ACC  |
               +------------| (clk, rst, en)|
                            +---------------+
```

Das Register unterbricht den rein kombinatorischen Signalpfad.
Zwischen zwei steigenden Flanken kann die ALU rechnen, aber das
Register übernimmt das Ergebnis erst an der nächsten gültigen Flanke.

## Neue Spade-Konzepte: `inst` und `port()`

Unsere Standard-ALU ist als `entity alu4_top(...)` implementiert.
Zum Einbauen einer `entity` wird das Schlüsselwort `inst` verwendet.
Eine `fn` wie `adders::add4(...)` würde dagegen ohne `inst`
aufgerufen werden.

Die bisherigen ALU-Ausgänge sind **`inv`-Parameter**:
`result: inv uint<4>`, `carry_out: inv bool` und `zero: inv bool`.
Bei der Instanziierung müssen diese Rückwärtsports beschaltet werden.

`port()` liefert dafür ein Paar `(normaler_Wert, inv_Port)`.
Der normale Wert wird in der übergeordneten Schaltung gelesen; der
invertierte Port wird an den Unterbaustein übergeben, der ihn mit
`set` beschaltet.

Ein isoliertes Beispiel, das **nicht die Akkumulator-Lösung** ist:

```spade
let (signal, rueckwaerts) = port();
// 'signal' kann gelesen werden, sobald ein Baustein 'rueckwaerts' setzt.
```

Für einen vollständigen ALU-Aufruf werden entsprechend alle drei
`inv`-Ausgänge verbunden. Von der ALU wird fürs Register nur
`result` benötigt; `carry_out` und `zero` bleiben in dieser
ersten Übung unbeobachtet. Später können wir sie separat speichern.

Siehe:
- [Spade: Instantiation](https://docs.spade-lang.org/reference/expressions/instantiation.html)
- [Spade: Inverted Signals and Wires](https://docs.spade-lang.org/wires.html)

## Sinnvolle Reihenfolge beim Implementieren

- Den aktuellen Registerwert als Ausgang `q` sichtbar machen.
- Einen ALU-Baustein instanziieren und dessen Ausgänge beschalten.
- Das ALU-Ergebnis als nächsten Registerwert bei `enable` verwenden.
- Synchronen Reset mit Vorrang berücksichtigen.
- Danach nur den Akkumulator testen, anschließend die Regressionstests.

Die Implementierung wurde inzwischen abgeschlossen und mit
**16.774/16.774 Prüfungen** getestet. Der Akkumulator kann nun als
öffentliche Entity in der Befehlsausführungsstufe verwendet werden.

## Testen

```sh
make test_accumulator4
```

Die C++-Testbench prüft alle **1024 Kombinationen aus gespeichertem
ACC-Wert, Operand und Operation**, inklusive vorheriger Herstellung des
Akkumulatorwertes, Pause bei ausgeschaltetem Enable, Reset-Priorität,
Eingangsänderungen ohne gültige Flanke und einer längeren
aufeinanderfolgenden Rechenkette.

VCD: `logic_basics/build/accumulator4.vcd`.
Der VS-Code-Standard-Testtask ruft nur diesen Test auf.
