# CPU-Kapitel: Der 4-Bit-Programmzähler (PC)

## 1. Was der PC speichert

Der **Program Counter (PC)** ist ein Register, das angibt, an welcher
Adresse die CPU ihren nächsten Befehl findet. Er speichert **keinen
Maschinenbefehl**, sondern eine Adresse. Den eigentlichen Befehl liefert
später ein Programmspeicher (ROM).

Unser PC ist vier Bit breit. Deshalb existieren genau 16 mögliche
Adressen, von 0 bis 15. Der spätere ROM kann beispielsweise
**16 Einträge zu je 6 Bit** enthalten; die Adresse ist also vier Bit
breit, während ein Maschinenbefehl sechs Bit besitzt.

```text
       PC (4 Bit)                 ROM (später)
      +----------+             +-------------------+
      | Adresse  | ----------> | 16 x 6-Bit-Wörter |
      +----------+             +-------------------+
                                     |
                                     v
                                instruction[5:0]
                                     |
                                     v
                             instruction_exec6_top
```

Ein PC ist kein Addierer, der kontinuierlich läuft: Sein **gespeicherter
Zustand** ändert sich nur bei der passenden Taktflanke.

## 2. Der Programmfluss

Im einfachsten Fall folgt auf Adresse 0 die 1, dann 2, 3 usw.
Bei vier Bit Breite gilt nach 15 wieder 0. Das heißt
**Modulo-16-Arithmetik**:

```text
... → 13 → 14 → 15 → 0 → 1 → ...
```

Damit Programme nicht immer nur geradeaus laufen, bereiten wir
zusätzlich einen *absoluten Sprung* (Jump) vor.
Mit `target` kann die nächste Adresse unmittelbar ausgewählt werden.

Unsere ISA hat bisher nur `ADD`, `AND`, `OR`, `XOR`.
Es gibt **noch keinen Maschinenbefehl für JUMP**. Die
Jump-Eingänge sind vorerst externe Steuersignale für die
PC-Übung; ihre Verknüpfung mit zukünftigen Befehlen entwerfen wir später.

## 3. Präzise Priorität der Steuersignale

Schnittstelle:

| Port | Typ | Zweck |
|---|---|---|
| `clk` | `clock` | Steigende Flanke als Aktualisierungszeitpunkt |
| `rst` | `bool` | Synchroner Reset auf Adresse 0 |
| `advance` | `bool` | Freigabe für den nächsten Befehlsort |
| `jump` | `bool` | Bei Freigabe statt +1 zur Zieladresse springen |
| `target` | `uint<4>` | Gewünschte Zieladresse, 0 bis 15 |
| `pc` | `inv uint<4>` | Aktuell **gespeicherte** Adresse |

An einer **steigenden Taktflanke**:

1. `rst = true`: Der PC wird **0**. Reset hat die höchste Priorität.
2. Sonst `advance = false`: PC bleibt gleich, selbst wenn `jump = true`.
3. Sonst `jump = true`: PC wird `target`.
4. Sonst: PC wird um 1 erhöht, bei 15 auf 0.

Wichtig: Ein Sprung wird nur mit `advance = true` ausgeführt.
So können wir später die gesamte CPU mit einem gemeinsamen
Schritt-/Pause-Signal anhalten. `jump` allein ist kein
Schreibzugriff auf das PC-Register.

### Kontrollbeispiele

| PC vorher | rst | advance | jump | target | PC nach Flanke |
|---:|---:|---:|---:|---:|---:|
| 7 | 0 | 1 | 0 | 12 | 8 |
| 15 | 0 | 1 | 0 | 8 | 0 |
| 7 | 0 | 1 | 1 | 12 | 12 |
| 7 | 0 | 0 | 1 | 12 | 7 |
| 7 | 1 | 0 | 1 | 12 | 0 |
| 7 | 1 | 1 | 0 | 12 | 0 |

`target` ist irrelevant, solange kein freigegebener Sprung stattfindet.

## 4. Wie ein PC sich von einem gewöhnlichen Zähler unterscheidet

Ein PC **ist** technisch eine Form von Register/Zähler. Seine
Architekturbedeutung ist jedoch speziell: Er steuert die
**Adresse des nächsten Befehls**. Deshalb hat er später
Verbindungen zu ROM, Sprungdecoder und CPU-Steuerwerk.

Der in `src/foundations/counters.spade` implementierte
`counter4_top` kann bisher nur hochzählen oder halten.
Hier erweitern wir genau dieses Prinzip um eine
**Zielauswahl für absolute Sprünge**.

Der PC selbst führt keine Befehle aus und verändert den Akkumulator
nicht. Er verwaltet ausschließlich die Befehlsadresse.

## 5. Spade: `reg`, `reset` und die nächste Adresse

Der ursprüngliche, noch unvollständige Top-Level der Übung war:

```spade
#[no_mangle(all)]
pub entity pc4_top(
    clk: clock,
    rst: bool,
    advance: bool,
    jump: bool,
    target: uint<4>,
    pc: inv uint<4>
) {
    // TODO: Zustand anhand der Steuersignale aktualisieren.
    reg(clk) current: uint<4> reset(rst: 0) = 0u4;
    set pc = current;
}
```

Der rechte Ausdruck bei `reg(clk)` bestimmt den Wert, der **an
der nächsten steigenden Flanke** gespeichert wird. `current`
bezeichnet dagegen den **momentan gespeicherten** Wert.

Der Reset gehört in das Register und ist bei uns **synchron**:
Wenn `rst` bei konstantem Takt von 0 auf 1 wechselt, darf
sich `pc` nicht sofort ändern.

### Baustein A: Der vorherige Registerwert

Unser alter Zähler enthält bereits die passende Struktur:

```spade
reg(clk) count: uint<4> reset(rst: 0) = if enable {
    count +. 1
} else {
    count
};
```

Hier ist `+.` Spades **überlaufende Addition** (Wrapping Add):
Die Summe bleibt in der vorgegebenen Bitbreite. Bei
`15 +. 1` wird aus vier Bit wieder `0`.
Die Form ist an unserem tatsächlich getesteten
`src/foundations/counters.spade` orientiert.

### Baustein B: Bedingte Auswahl

`if`-Ausdrücke in Spade ergeben selbst einen Wert. Du kannst
sie deshalb schachteln. Ein isoliertes, von unserem PC unabhängiges
Beispiel für die Priorität zweier Bool-Signale:

```spade
let selected = if use_first {
    first_value
} else if use_second {
    second_value
} else {
    fallback
};
```

Für den PC musst du entscheiden, welcher der bereits vorhandenen
Signale diese Auswahl steuert. Den fertig zusammengesetzten
Registerausdruck nehmen wir bewusst nicht vorweg.

### Baustein C: Den gespeicherten Wert ausgeben

`pc` ist ein `inv uint<4>`-Ausgang. Wie beim Akkumulator
wird er innerhalb der Entity mit `set` verbunden:

```spade
set pc = current;
```

Daran brauchst du in dieser Übung nichts zu verändern.
Es darf ausdrücklich nicht der *nächste berechnete* Wert ausgegeben
werden, sondern der bisher **gespeicherte**.

Es sind keine `inst`- oder `port()`-Aufrufe erforderlich:
Diese Übung modelliert das PC-Register als eigenständige Entity.

## 6. Zeitliche Bedeutung beim späteren CPU-Zusammenbau

Ein wichtiger Punkt für die nächste Übung:
Bei einer CPU mit *kombinatorischem* ROM werden
`PC` und `ACC` bei derselben steigenden Taktflanke
aus ihren **alten** Eingangswerten aktualisiert.

Angenommen, vor der Flanke gilt:

- `PC = 3`: Der ROM liefert den Befehl an Adresse 3.
- `ACC = 5`: Die Ausführungsstufe verarbeitet diesen Befehl.
- `advance = true`: Der PC soll anschließend auf 4 wechseln.

An der Flanke speichert der Akkumulator das Ergebnis des
**Befehls von Adresse 3**, während der PC auf **4** steigt.
Danach darf die kombinatorische ROM-Ausgabe auf den Befehl
an Adresse 4 wechseln. Dieser wird aber erst bei der
nächsten steigenden Flanke übernommen.

Damit tritt bei korrekt synchronem Aufbau kein zufälliger
Off-by-one-Fehler auf. **Voraussetzung** sind passende
Setup-/Hold-Zeiten und ein ROM, das sein Wort
kombinatorisch aus der PC-Adresse bereitstellt.
Ein *synchroner* ROM kann ein anderes Timing erfordern,
das wir dann gesondert betrachten.

### Warum `advance` und `enable` später zusammengehören

Die Befehlsausführungsstufe besitzt `enable`,
der PC besitzt `advance`.
Für eine gemeinsame Schrittausführung möchten wir
normalerweise, dass **beide gleichzeitig freigegeben** sind.
Andernfalls kann der PC weiterlaufen, während der ACC
keine Befehle verarbeitet, oder umgekehrt.

## 7. Testgetriebene Aufgabe

Die Zustandsübergangslogik in `src/cpu/pc4.spade` ist inzwischen
implementiert. Die Testbench prüft sie gegen ein unabhängiges Softwaremodell.

`make test_pc4` prüft:

- Alle 16 möglichen Startadressen gegen alle 16 Sprungziele.
- Reset-Priorität, insbesondere mit gleichzeitig aktivem Jump.
- Keine Bewegung bei `advance=false`.
- Fortlaufendes Zählen und Überlauf bei 15.
- Keine Zustandsänderung durch Signalwechsel bei hohem Takt
  oder fallende Flanken.
- Eine längere gemischte Folge mit Sprüngen, Pausen und Reset.

Die Testbench nutzt ein unabhängiges Softwaremodell
und vergleicht den PC nach jedem relevanten Simulationsschritt.
Die PC-Implementierung hat **12.166/12.166 Zustandsprüfungen** bestanden.
Eine Signalaufzeichnung entsteht unter
`logic_basics/build/pc4.vcd`.

Nach weiteren Änderungen zuerst `make test_pc4` und danach
`make test` ausführen. `make test_completed` berücksichtigt jetzt
auch den abgeschlossenen PC-Test.

## 8. Wohin führt das?

Mit einem PC wissen wir erstmals, **welcher Befehl an der Reihe
ist**. Als Nächstes ergänzen wir einen kleinen Programmspeicher
mit 16 Befehlen zu je 6 Bit und verbinden ihn mit
`instruction_exec6_top`.

Damit besitzen wir einen einfachen Fetch-/Execute-Zyklus
für sequenzielle Befehle. Sprünge bleiben zunächst
ein externer Testmodus, bis unsere ISA um passende
Befehlsarten erweitert wird. Der Emulator bekommt dann
neben seinem Akkumulator eine PC-Zustandsvariable
und kann tatsächliche Programme Schritt für Schritt
durchlaufen.
