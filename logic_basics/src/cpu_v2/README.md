# CPU v2: Eigenständige zweite CPU-Generation

Hier entsteht die zweite Version unserer **4-Bit-CPU mit 8-Bit-Befehlen**.

Die abgeschlossene erste Generation bleibt unverändert unter
`logic_basics/src/cpu/`. Insbesondere bleiben `cpu4_top`, ihr
6-Bit-Befehlsformat und die vorhandenen Testbenches als Referenz erhalten.

## Geplanter Aufbau

Die folgenden Dateien werden **erst mit der jeweiligen Übung** angelegt:

```text
src/cpu_v2/
├── README.md        # Architektur und Abgrenzung (bereits vorhanden)
├── main.spade       # Modulregistrierung (geplant)
├── decoder8.spade   # 8-Bit-Befehle interpretieren (geplant)
├── control8.spade   # ACC-/PC-Steuerung ableiten (geplant)
├── rom16x8.spade    # 16 feste 8-Bit-Befehle (geplant)
└── cpu_v2.spade     # Neuer CPU-Top-Level (geplant)
```

Die genaue Aufteilung kann sich mit dem ISA-Entwurf noch ändern.
Eine vorhandene ALU dürfen wir gegebenenfalls wiederverwenden; wir
kopieren sie nicht unnötig. Alte Schnittstellen werden dabei nicht
für die neue Version umgeschrieben.

Die Spezifikation liegt unter [`docs/cpu_v2/isa.md`](../../docs/cpu_v2/isa.md).

## Entwicklungsregeln

- Eigenständige neue Spade-Module und eindeutige Namen wie `cpu_v2_top`.
- Neue Tests mit eigenen Namen, beispielsweise `test_decoder8` und
  `test_cpu_v2`; bestehende Tests unverändert lassen.
- Benannte `inst { port: signal }`-Verbindungen gemäß `AGENTS.md`.
- Neue Aufgaben zunächst anhand von Theorie, unabhängigem Softwaremodell
  und fehlschlagenden Tests beschreiben; die eigentliche Spade-Lösung
  entsteht in der jeweiligen Übung.
- Noch **kein `main.spade`** und kein Eintrag in `src/main.spade`:
  Diese Phase dokumentiert zunächst nur die Architektur. So bleibt
  der bisherige Build unberührt.
