# CPU v2: Eigenständige zweite CPU-Generation

Hier entsteht die zweite Version unserer **4-Bit-CPU mit 8-Bit-Befehlen**.

Die abgeschlossene erste Generation bleibt unverändert unter
`logic_basics/src/cpu/`. Insbesondere bleiben `cpu4_top`, ihr
6-Bit-Befehlsformat und die vorhandenen Testbenches als Referenz erhalten.

## Geplanter Aufbau

Die Verzeichnisstruktur wird schrittweise mit den Übungen aufgebaut:

```text
src/cpu_v2/
├── README.md        # Architektur und Abgrenzung (bereits vorhanden)
├── main.spade       # Modulregistrierung (vorhanden)
├── decoder8.spade   # 8-Bit-Felddecoder (abgeschlossen)
├── control8.spade   # Fertige fn für die Steuerlogik + Testhülle
├── state_v2.spade   # PC-/ACC-Zustandssteuerung (offene TDD-Übung)
├── rom16x8.spade    # 16 feste 8-Bit-Befehle (geplant)
└── cpu_v2.spade     # Neuer CPU-Top-Level (geplant)
```

Die genaue Aufteilung kann sich mit dem ISA-Entwurf noch ändern.
Eine vorhandene ALU dürfen wir gegebenenfalls wiederverwenden; wir
kopieren sie nicht unnötig. Alte Schnittstellen werden dabei nicht
für die neue Version umgeschrieben.

Die Spezifikation einschließlich eines konkreten Countdown-/LED-Alarm-Programms
liegt unter [`docs/cpu_v2/isa.md`](../../docs/cpu_v2/isa.md).
Der Felddecoder ist abgeschlossen: [`docs/cpu_v2/decoder8.md`](../../docs/cpu_v2/decoder8.md).
Die ebenfalls abgeschlossene Steuerlogik ist dokumentiert unter
[`docs/cpu_v2/control8.md`](../../docs/cpu_v2/control8.md). Die wiederverwendbare
`pub fn decode_control_signals` ist rein kombinatorisch; nur die
`test_control8_top`-Entity stellt vier Ausgänge für Verilator bereit.

Die aktuelle Übung behandelt die Zustandsübergänge von PC und ACC:
[`docs/cpu_v2/state_v2.md`](../../docs/cpu_v2/state_v2.md).
Dazu gibt es den Platzhalter `state_v2.spade` und die unabhängige
Testbench `test/state_v2.cc` (`make test_state_v2`).

Den geplanten Befehlssatz können wir **vor der Spade-Implementierung**
mit `make demo_cpu_v2` im Repository-Root ausprobieren. Das eigenständige
C++-Referenzmodell steht unter [`test/cpu_v2_isa_demo.cc`](../../test/cpu_v2_isa_demo.cc).
Es druckt einen Maschinenbefehl-/Zustands-Trace und prüft den Programmablauf.
Der Softwaretest ist kein Test einer bereits vorhandenen CPU-v2-Hardware.

## Entwicklungsregeln

- Eigenständige neue Spade-Module und eindeutige Namen wie `cpu_v2_top`.
- Neue Tests mit eigenen Namen, beispielsweise `test_decoder8` und
  `test_cpu_v2`; bestehende Tests unverändert lassen.
- Benannte `inst { port: signal }`-Verbindungen gemäß `AGENTS.md`.
- Neue Aufgaben zunächst anhand von Theorie, unabhängigem Softwaremodell
  und fehlschlagenden Tests beschreiben; die eigentliche Spade-Lösung
  entsteht in der jeweiligen Übung.
- `src/main.spade` bindet `cpu_v2/main.spade` ein. Felddecoder und
  Steuerfunktion sind abgeschlossen. Der gemeldete Testlauf nach dem
  `fn`-Refactoring ergab 144/144 Vergleiche.
- `make test_completed` enthält die 20 abgeschlossenen Hardwaretests;
  `make test` umfasst zusätzlich die offene Zustandssteuerung.
  CPU v1 bleibt unverändert.
