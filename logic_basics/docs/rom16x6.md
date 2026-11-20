# CPU-Datenpfad III: 16 x 6-Bit-Programmspeicher (ROM)

## 1. Was diese Übung ergänzt

Der fertige 4-Bit-Program-Counter (PC) wählt eine **Adresse von 0 bis 15**.
Unser 6-Bit-Decoder und die Ausführungsstufe verstehen bereits Maschinenbefehle.
Dazwischen fehlt noch der Baustein, der aus einer Adresse einen Befehl macht.

```text
     PC (4 Bit)                ROM (16 x 6 Bit)          Execute (später)
   +-------------+           +-----------------+       +------------------+
   |  address    | --------> | Befehl je Adresse| ----> | instruction_exec6|
   +-------------+           +-----------------+       +------------------+
                              instruction (6 Bit)
```

Der **ROM** (Read-Only Memory) stellt für jede Adresse ein fest definiertes
6-Bit-Wort bereit. Die CPU kann ihn in dieser Übung nicht überschreiben.
Der ROM interpretiert die Befehle nicht; er liefert lediglich ihre Bits.

Wir modellieren zuerst nur den **eigenständigen ROM**, noch nicht die
Verdrahtung mit PC oder Akkumulator.

## 2. Die Schnittstelle: Lesen ohne Takt

`rom16x6_top` in `src/cpu/rom16x6.spade` besitzt genau zwei Ports:

| Port | Spade-Typ | Bedeutung |
|---|---|---|
| `address` | `uint<4>` | Adresse 0 bis 15 |
| `instruction` | `inv uint<6>` | Maschinenbefehl an dieser Adresse |

Es gibt hier **kein `clk`, `rst`, `reg` oder `advance`**.
Der ROM dieser Übung ist *kombinatorisch*: Nach einem Adresswechsel folgt die
Ausgabe der neuen Adresse, sobald sich die Logik stabilisiert hat. In der
Simulation wird dazu nur `eval()` benötigt, keine Taktflanke.

Ein anderer möglicher Aufbau wäre ein **synchroner ROM** mit getaktetem
Leseausgang. Der hätte ein anderes Zeitverhalten und wird hier bewusst
nicht verwendet.

## 3. Verbindlicher Programminhalt

Die vier oberen Befehlstabellen-Bits? Nein: Unser ISA-Format bleibt
`instruction[5:4] = op` (zwei Bits) und
`instruction[3:0] = operand` (vier Bits).

| Adresse (dez.) | Befehl (binär) | Hex | Bedeutung |
|---:|---|---:|---|
| 0 | `10_0101` | `0x25` | OR 5 |
| 1 | `00_1011` | `0x0B` | ADD 11 |
| 2 | `11_0111` | `0x37` | XOR 7 |
| 3 | `01_0011` | `0x13` | AND 3 |
| 4 | `00_0001` | `0x01` | ADD 1 |
| 5 | `11_1111` | `0x3F` | XOR 15 |
| 6 | `10_1000` | `0x28` | OR 8 |
| 7 | `01_0110` | `0x16` | AND 6 |
| 8 | `00_1110` | `0x0E` | ADD 14 |
| 9 | `11_0010` | `0x32` | XOR 2 |
| 10 | `10_0100` | `0x24` | OR 4 |
| 11 | `00_0011` | `0x03` | ADD 3 |
| 12 | `01_1001` | `0x19` | AND 9 |
| 13 | `11_1011` | `0x3B` | XOR 11 |
| 14 | `10_0010` | `0x22` | OR 2 |
| 15 | `00_0101` | `0x05` | ADD 5 |

Das ist ein **festes Testprogramm**, kein allgemeiner ROM-Inhalt jeder CPU.
Alle 16 Worte sind unterschiedlich, damit Vertauschungen und
Adressierungsfehler auffallen. Die ersten vier Befehle entsprechen
dem Handbeispiel aus `instruction_exec6.md`.

Kontrollbeispiele:

- `address = 0` ergibt `instruction = 10_0101` (OR 5).
- `address = 3` ergibt `instruction = 01_0011` (AND 3).
- `address = 15` ergibt `instruction = 00_0101` (ADD 5).

Beim späteren PC-Überlauf von 15 auf 0 wird wieder das erste
Speicherwort ausgewählt. Diese Umlauflogik gehört aber zum **PC**,
nicht in den ROM.

## 4. Hardwaremodell: Auswahl, nicht Speicherung zur Laufzeit

Du kannst dir den ROM als eine feste **Wahrheitstabelle** vorstellen:

```text
address[3:0] ───> [ feste Zuordnung / Auswahl ] ───> instruction[5:0]
```

Die 16 Speicherworte sind beim Entwurf definiert. Der Compiler bzw.
die Synthese kann das als Auswahl- und Logiknetz realisieren. Die
Beschreibung garantiert **nicht**, dass ein FPGA dafür einen
dedizierten Block-RAM verwendet. Unser erster ROM ist klein und
rein kombinatorisch.

## 5. Spade: `match` für vollständige Zuordnungen

Du kennst `match` bereits aus der ALU und Ampel-FSM. Bei diesem ROM
wählt es anhand einer vierbittigen Adresse genau einen sechsbittigen Wert.
Ein `match` ist ein Ausdruck; jeder Zweig liefert einen Wert.

Ein **unabhängiges Mini-Beispiel**, nicht die ROM-Lösung:

```spade
fn pick_color(code: uint<2>) -> uint<3> {
    match code {
        0 => 0b101u3,
        1 => 0b010u3,
        2 => 0b111u3,
        3 => 0b000u3
    }
}
```

Wichtig ist die **Vollständigkeit**: Jeder mögliche Eingabewert muss
zu einem Zweig passen, sonst meldet Spade einen Fehler. Für
`uint<4>` sind das 16 Adressen. Ein `_`-Zweig wäre ebenfalls
möglich, würde hier aber leicht einen versehentlich ausgelassenen
Eintrag verschleiern.

Bei unserer Entity muss das ausgewählte Wort wie beim Decoder
über den invertierten Ausgang mit `set` verbunden werden.
`instruction` ist ein `inv`-Port, kein Speicherregister.

Spade-Referenz: [Match-Ausdrücke](https://docs.spade-lang.org/reference/expressions/match.html).

## 6. Deine Aufgabe

Der vorbereitete Platzhalter lautet:

```spade
#[no_mangle(all)]
pub entity rom16x6_top(
    address: uint<4>,
    instruction: inv uint<6>
) {
    // TODO: Alle 16 Adressen anhand der Tabelle abbilden.
    set instruction = 0u6;
}
```

Implementiere die Tabelle selbst in `src/cpu/rom16x6.spade`.

Dabei soll gelten:

1. Adresse auswählen, das zugehörige **konstante** 6-Bit-Wort liefern.
2. Jeden der 16 Adresswerte genau wie in der Tabelle behandeln.
3. Keine zusätzliche Taktlogik und keine Änderungen an PC oder Decoder.
4. Das Ergebnis ausschließlich an `instruction` ausgeben.

Du kannst die Werte als Hex-Literale wie `0x25u6` oder
Binär-Literale wie `0b100101u6` schreiben. Beide repräsentieren
denselben 6-Bit-Wert; die Trennung `10_0101` in der Tabelle zeigt
nur das ISA-Format.

## 7. Testgetriebene Entwicklung

Im Repository-Root:

```sh
make test_rom16x6
```

Die unabhängige C++-Testbench in `test/rom16x6.cc` prüft:

- Alle 16 Adressen in aufsteigender und absteigender Reihenfolge.
- Alle 256 Übergänge von jeder Adresse zu jeder anderen Adresse.
- Eine weitere deterministische Folge von 64 Adresswechseln.

Das sind **352 Ausgabevergleiche**. Der Platzhalter ist absichtlich
falsch; der Test schlägt bis zu deiner Implementierung fehl.
Die Signale werden in `logic_basics/build/rom16x6.vcd` aufgezeichnet.

`make test_completed` prüft weiterhin nur die bereits fertigen
Übungen. Nach erfolgreichem ROM-Test anschließend `make test`
für die vollständige Regression aufrufen.

## 8. Danach: PC, ROM und Execute verbinden

Wenn der ROM funktioniert, verbinden wir:

- `pc4_top` als Adressquelle,
- `rom16x6_top` als Befehlsspeicher,
- `instruction_exec6_top` als Ausführungsstufe.

Bei einem **kombinatorischen ROM** liegt vor der nächsten steigenden
Taktflanke der Befehl der aktuell gespeicherten PC-Adresse an.
An der Flanke übernehmen PC und Akkumulator unabhängig ihre nächsten
Zustände. Die ersten vier ROM-Einträge lassen sich dann direkt mit
dem Beispiel aus `instruction_exec6.md` vergleichen.

Sprünge bleiben vorerst externe Steuersignale; ein JUMP-Opcode ist in
unserer bisherigen 6-Bit-ISA noch nicht vorgesehen.
