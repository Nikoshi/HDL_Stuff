# CPU v2: Eigenständige zweite CPU-Generation

Hier liegt die zweite Version unserer **4-Bit-CPU mit 8-Bit-Befehlen**.

Die abgeschlossene erste Generation bleibt unverändert unter
`logic_basics/src/cpu/`. Insbesondere bleiben `cpu4_top`, ihr
6-Bit-Befehlsformat und die vorhandenen Testbenches als Referenz erhalten.

## Aufbau

Die Bausteine liegen getrennt von CPU v1:

```text
src/cpu_v2/
├── README.md        # Architektur und Abgrenzung (bereits vorhanden)
├── main.spade       # Modulregistrierung (vorhanden)
├── decoder8.spade   # 8-Bit-Felddecoder (abgeschlossen)
├── control8.spade   # Fertige fn für die Steuerlogik + Testhülle
├── state_v2.spade   # PC-/ACC-Zustandssteuerung (abgeschlossen)
├── rom16x8.spade    # 16 feste 8-Bit-Befehle (abgeschlossen)
└── cpu_v2.spade     # Fertiger Integrations-Top-Level
```

Die 4-Bit-ALU aus CPU v1 wird über das öffentliche Modul
`lib::cpu::alu` wiederverwendet. Die CPU-v1-Schnittstellen bleiben unverändert.

Die Spezifikation einschließlich eines konkreten Countdown-/LED-Alarm-Programms
liegt unter [`docs/cpu_v2/isa.md`](../../docs/cpu_v2/isa.md).
Der Felddecoder ist abgeschlossen: [`docs/cpu_v2/decoder8.md`](../../docs/cpu_v2/decoder8.md).
Die ebenfalls abgeschlossene Steuerlogik ist dokumentiert unter
[`docs/cpu_v2/control8.md`](../../docs/cpu_v2/control8.md). Die wiederverwendbare
`pub fn decode_control_signals` ist rein kombinatorisch; nur die
`test_control8_top`-Entity stellt vier Ausgänge für Verilator bereit.

Die PC-/ACC-Zustandssteuerung ist abgeschlossen:
[`docs/cpu_v2/state_v2.md`](../../docs/cpu_v2/state_v2.md).
Der gemeldete Verilator-Lauf bestand 69.635/69.635 Vergleiche.

Der 16×8-Bit-Programmspeicher ist ebenfalls fertig:
[`docs/cpu_v2/rom16x8.md`](../../docs/cpu_v2/rom16x8.md).
Die unabhängige Testbench `test/cpu_v2/rom16x8.cc` bestand gemeldete
608/608 Vergleiche (`make test_rom16x8`).

Die abgeschlossene Integration verbindet die fertigen Bausteine:
[`docs/cpu_v2/cpu_v2.md`](../../docs/cpu_v2/cpu_v2.md).
Der Top-Level `cpu_v2.spade` und die unabhängige Testbench
`test/cpu_v2/cpu_v2.cc` gehören zum Target `make test_cpu_v2`.
Der gemeldete Lauf bestand **3.691/3.691 Zustandsvergleiche**.

Das vollständige Demo-Programm lässt sich unabhängig von der Hardware
mit `make demo_cpu_v2` im Repository-Root ausprobieren. Das eigenständige
C++-Referenzmodell steht unter [`test/cpu_v2/cpu_v2_isa_demo.cc`](../../test/cpu_v2/cpu_v2_isa_demo.cc).
Es druckt einen Maschinenbefehl-/Zustands-Trace und prüft den Programmablauf.
Der Softwaretest bleibt ein unabhängiger Vergleichspunkt für die integrierte Hardware.

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
- `make test_completed` enthält die 23 abgeschlossenen Hardwaretests;
  `make test` führt dieselbe Regression aus.
  CPU v1 bleibt unverändert.
