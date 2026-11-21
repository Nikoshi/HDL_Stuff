# FSM-Übung: Vierphasige Ampel

## Lernziel

Eine Moore-FSM aus drei Bausteinen zusammensetzen: Zustandsregister,
Übergangslogik und Ausgangsdekodierung. Die C++-Testbench legt das
geforderte Verhalten unabhängig von der Spade-Implementierung fest.

## Zustände und Ausgänge

| Zustand | red | yellow | green |
|---|---:|---:|---:|
| `Red` | 1 | 0 | 0 |
| `RedYellow` | 1 | 1 | 0 |
| `Green` | 0 | 0 | 1 |
| `Yellow` | 0 | 1 | 0 |

`advance` steuert bei einer **steigenden** Taktflanke den Übergang:

```text
Red -> RedYellow -> Green -> Yellow -> Red
```

Ohne `advance` bleibt der Zustand unverändert. `rst` setzt die FSM
synchron auf `Red`, unabhängig von `advance`. Ein Zustandswechsel
durch Reset erfolgt daher ebenfalls erst an der nächsten steigenden Flanke.

Die Ausgänge hängen **nur vom gespeicherten Zustand** ab, nicht unmittelbar
von `advance` oder `rst`. Das macht sie zu Moore-Ausgängen.

## Spade-Grundlagen

```spade
// Ein Beispiel für einen benannten Zustandstyp.
enum Color {
    Red,
    Yellow,
    Green
}

// Jeder mögliche Zustand muss durch einen match-Zweig abgedeckt werden.
fn is_green(c: Color) -> bool {
    match c {
        Color::Red => false,
        Color::Yellow => false,
        Color::Green => true
    }
}
```

Die eigene Implementierung nutzt den vorgegebenen Typ `TrafficState`.
Im `reg(clk)`-Ausdruck muss **der nächste Zustand** berechnet werden.
Eine davon getrennte Kombinatorik erzeugt die drei Lampensignale aus dem
**aktuellen Zustand**.

Hilfreiche Unterscheidung:

- `if advance { ... } else { ... }` entscheidet, ob ein Zustandswechsel erfolgt.
- `match state { ... }` kann den nächsten Zustand oder die Ausgänge bestimmen.
- `reg(clk) ... reset(rst: ...)` speichert den Zustand und priorisiert Reset.

## Beispiel zum Nachdenken

Angenommen die Ampel steht in `Green`. `advance` wechselt bei dauerhaft
hohem Takt von 0 auf 1: Die Lampen dürfen sich dabei nicht ändern.
Erst nach einer fallenden und danach einer steigenden Taktflanke
ist der Wechsel zu `Yellow` erlaubt.

Ein zweiter Fall: Die Ampel steht auf `Yellow` und sowohl `rst` als
auch `advance` sind bei der nächsten steigenden Flanke aktiv.
Der nächste Zustand muss `Red` sein.

## Testen

```sh
make test_traffic_light
```

Die Testbench testet mehrere vollständige Umläufe, Ruhephasen,
Eingangswechsel zwischen Taktflanken und Reset aus jeder Phase.
Die VCD-Datei heißt `logic_basics/build/traffic_light.vcd`.

Der Ampel-Test ist weiterhin über `make test_traffic_light` und den VS-Code-Einzeltest verfügbar. Solange die CPU-Ausführungsübung offen ist, überprüft `make test_completed` die abgeschlossenen Bausteine.

---

[← Zurück zur Übersicht](../../README.md) · [Weiter: 4-Bit-ALU →](alu4.md)
