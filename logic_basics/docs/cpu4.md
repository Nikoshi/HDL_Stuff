# CPU-Datenpfad IV: PC, ROM und Execute zusammensetzen

## 1. Lernziel

Bisher wurden `pc4_top`, `rom16x6_top` und
`instruction_exec6_top` einzeln implementiert und getestet.
Jetzt kombinieren wir **diese vorhandenen Entities** zu einer
minimalen CPU mit fest eingebautem 16-Wort-Programm.

Die neue Entity heißt `cpu4_top`. Sie hat **kein zusätzliches
Register**: Der Zustand befindet sich bereits in PC und Akkumulator.

```text
      +-----------+  4 Bit   +-------------+   6 Bit  +-------------------+
      |  pc4_top  | -------> | rom16x6_top | -------> |instruction_exec6  |
      |    PC     |          |  Programm   |          | Decoder/ALU/ACC   |
      +-----------+          +-------------+          +-------------------+
           ^                                                  ^
           |                   clk, rst, advance              |
           +--------------------------------------------------+
```

`advance` geht an PC und Ausführungsstufe, damit eine Pause
beide Zustände gleichzeitig anhält. Der ROM hat keinen Takt.

## 2. Verbindliche Top-Level-Schnittstelle

Datei: `logic_basics/src/cpu/cpu4.spade`

| Port | Typ | Bedeutung |
|---|---|---|
| `clk` | `clock` | Gemeinsamer Takt |
| `rst` | `bool` | Synchroner Reset von PC und ACC |
| `advance` | `bool` | Gemeinsames Schritt-/Pause-Signal |
| `jump` | `bool` | Externe Sprunganforderung an den PC |
| `target` | `uint<4>` | Absolute Sprungadresse |
| `pc` | `inv uint<4>` | Gegenwärtig gespeicherte Befehlsadresse |
| `instruction` | `inv uint<6>` | Aktuell am ROM anliegender Befehl |
| `acc` | `inv uint<4>` | Gegenwärtig gespeicherter Akkumulator |

`pc`, `instruction` und `acc` sind zur Beobachtung nach außen
geführt. Die Instruktion wird nicht zusätzlich gespeichert.

Die ISA ist unverändert: `00` ADD, `01` AND, `10` OR,
`11` XOR; der Operand ist jeweils ein 4-Bit-Immediate.
Es gibt **noch keinen JUMP-Maschinenbefehl**.
`jump` und `target` steuern weiterhin nur von außen den PC.

## 3. Zustand und Zeitverhalten

Der aktuelle CPU-Zustand ist das Paar `(PC, ACC)`.
Der ROM bildet den aktuellen PC **kombinatorisch** auf einen Befehl ab:

```text
instruction = ROM[PC]
```

An jeder **steigenden Taktflanke** ist die folgende
Fallunterscheidung verbindlich:

```text
PC_neu =
    0                 falls rst
    PC                falls !rst && !advance
    target            falls !rst && advance && jump
    (PC + 1) mod 16   sonst

ACC_neu =
    0                               falls rst
    ACC                             falls !rst && !advance
    ALU(ACC, ROM[PC])               sonst
```

Alle Werte auf der rechten Seite sind die Werte **vor der Flanke**.
Damit beeinflusst die neue PC-Adresse erst den Befehl, der
**nach** der Flanke am ROM-Ausgang anliegt. Der Akkumulator
verarbeitet an derselben Flanke noch `ROM[PC_alt]`.

### Konkreter Ablauf

```text
Vor Flanke 1: PC=0, instruction=OR 5,   ACC=0
Nach Flanke 1: PC=1, instruction=ADD 11, ACC=5

Vor Flanke 2: PC=1, instruction=ADD 11, ACC=5
Nach Flanke 2: PC=2, instruction=XOR 7,  ACC=0
```

Für die zweite Flanke gilt `5 + 11 = 16` und bei vier Bit
bleibt `ACC = 0`. Der bei `PC=2` sichtbare XOR-Befehl
wird **noch nicht** an Flanke 2 ausgeführt.

### Pause, Reset und Sprung

| Vor Flanke | `rst` | `advance` | `jump` | Wirkung |
|---|---:|---:|---:|---|
| `PC=3, ACC=7` | 0 | 0 | 0 | Beide Register halten |
| `PC=3, ACC=7` | 0 | 0 | 1 | Beide halten, Jump wird ignoriert |
| `PC=3, ACC=7` | 1 | 0 | 1 | Beide Register gehen auf 0 |
| `PC=3, ACC=7` | 0 | 1 | 1 | Befehl an Adresse 3 ausführen, PC auf `target` setzen |

Das letzte Beispiel ist bewusst wichtig: Ein externer Jump
**unterdrückt nicht** die Ausführung des aktuell ausgewählten
Befehls. Er bestimmt nur, von welcher Adresse der **folgende**
Befehl geholt wird. Erst eine spätere Erweiterung der ISA
kann daraus echte Jump-Instruktionen mit eigenen Regeln machen.

Außerhalb der steigenden Flanke dürfen `rst`, `advance`,
`jump` und `target` den Zustand nicht verändern.
Nach einer Zustandsänderung darf `instruction` hingegen
kombinatorisch auf den neuen PC-Wert wechseln.

## 4. Spade: Entities verschalten, nicht neu schreiben

Im aktuellen Repository sind diese drei Import-Pfade bereits vorhanden:

```spade
use lib::cpu::pc4;
use lib::cpu::rom16x6;
use lib::cpu::instruction_exec6;
```

Die **Argumentreihenfolge** der vorhandenen Entities:

```text
pc4_top(clk, rst, advance, jump, target, pc)
rom16x6_top(address, instruction)
instruction_exec6_top(clk, rst, enable, instruction, q)
```

Mit der im Projekt gepinnten Spade-Version verwenden wir für
`inst` Positionsargumente. Zur Verbindung eines `inv`-Ausgangs
mit einem lesbaren Signal dient wieder `port()`, zum Beispiel:

```spade
// Unabhängiges Muster für eine Verbindung:
let (received_value, output_wire) = port();
```

`received_value` ist die **lesbare** Seite;
`output_wire` wird als Argument an den `inv`-Port
einer Unter-Entity übergeben.

Eine mögliche gedankliche Zerlegung deiner Aufgabe:

1. **PC-Instanz:** Gespeicherten PC über ein `port()`-Paar auslesen.
2. **ROM-Instanz:** Dessen Adress-Eingang mit dem gelesenen PC verbinden;
   den ROM-Ausgang über ein weiteres `port()`-Paar lesen.
3. **Execute-Instanz:** Den ROM-Befehl und die gemeinsamen
   `clk`-, `rst`- und `advance`-Signale anschließen;
   den Akkumulatorwert über ein drittes `port()`-Paar abgreifen.
4. **Top-Level-Ausgänge:** Die drei lesbaren Werte über `set`
   an `pc`, `instruction` und `acc` legen.

**Achtung:** `instruction_exec6_top` heißt das Schrittsignal
`enable`, während der PC es `advance` nennt. Das ist
derselbe gewünschte Taktfreigabe-Wert, nicht zwei unabhängige
Steuersignale.

Die vollständige Verdrahtung bleibt deine Übung.
Kopiere weder die ROM-Tabelle noch ALU-, Decoder- oder PC-Logik
in dieses neue Modul. Ein weiteres `reg` wäre hier ein
Architekturfehler, weil es das vereinbarte Timing verändern würde.

## 5. TDD-Aufgabe

Der kompilierbare Platzhalter befindet sich in
`src/cpu/cpu4.spade`. Er gibt zunächst nur Nullen aus
und ist deshalb absichtlich noch **keine** CPU.

Im Repository-Root:

```sh
make test_cpu4
```

Die neue C++-Testbench in `test/cpu4.cc` enthält ein
**unabhängiges Softwaremodell** für PC, ROM-Programm und ACC.
Es vergleicht nach den Simulationsschritten alle drei
Ausgänge `pc`, `instruction`, `acc`.

Der Test prüft:

- Die ersten 32 normalen Programmschritte (einschließlich PC-Überläufen).
- Alle 16 PC-Adressen zusammen mit allen 16 Sprungzielen.
- Pause, Reset-Priorität und Sprünge bei deaktiviertem `advance`.
- Änderungen der Steuersignale bei bereits hohem Takt und fallende Flanken.
- 256 weitere gemischte Schritte mit Pausen, Resets und Sprüngen.

Das ergibt **6.278 Zustandsvergleiche**, sobald die Implementierung
korrekt ist. Der aktuelle Platzhalter soll den Test nicht bestehen.
Die Signalverläufe liegen in `logic_basics/build/cpu4.vcd`.

`make test_completed` lässt die noch offene CPU-Übung aus.
Nach bestandenem `make test_cpu4` folgt `make test`
für die vollständige Regression.

## 6. Was danach möglich wird

Wenn dieser Top-Level funktioniert, kann die Lern-CPU ein
vollständiges festes Programm aus dem ROM ausführen. Ein
Softwaremodell kann PC und ACC Schritt für Schritt verfolgen.

Es gibt weiterhin Einschränkungen: kein Datenspeicher,
kein Befehlsregister, keine automatische Halt-Bedingung,
kein Sprungopcode und keine veränderbaren Programme.
Das sind mögliche **spätere** Architekturthemen, nicht Teil
dieser Verdrahtungsübung.
