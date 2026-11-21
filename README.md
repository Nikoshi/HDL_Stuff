# HDL_Stuff

Digitaltechnik-Experimente mit [Spade](https://docs.spade-lang.org/).
Von einfachen Schaltungen bis zu kleinen CPUs, getestet mit Verilator.

## Setup

Unter Linux und macOS mit [Nix](https://nixos.org/):

```sh
git clone https://github.com/Nikoshi/HDL_Stuff.git
cd HDL_Stuff
nix develop
```

Die Dev-Shell bringt Spade, Swim, Verilator, Yosys und Make mit.
Unter Linux ist auch Surfer dabei.

## Bauen und testen

```sh
make logic_basics     # Spade übersetzen
make test_completed   # Abgeschlossene Übungen
make test             # Alle Tests, inklusive der aktuellen Übung
make test_rom16x8     # CPU-v2-Programmspeicher testen
make test_cpu_v2      # Offene CPU-v2-Integrationsübung
make demo_cpu_v2      # ISA-Demo im Softwaremodell
make help             # Alle verfügbaren Targets
```

Die Testbench-Traces liegen unter `logic_basics/build/`.
`make clean` entfernt das Verzeichnis samt VCD-Dateien.

## Projektstand

Die erste 4-Bit-CPU ist abgeschlossen. CPU v2 verwendet weiterhin
4-Bit-Register, aber 8-Bit-Befehle. Decoder, Steuerlogik, Zustandsregister und
[16×8-Bit-Programmspeicher](logic_basics/docs/cpu_v2/rom16x8.md)
sind fertig. Aktuelle Übung: [CPU-v2-Integration](logic_basics/docs/cpu_v2/cpu_v2.md).

## Code und Lernnotizen

- `logic_basics/src/foundations/`: Logikgrundlagen
- `logic_basics/src/cpu/`: erste CPU
- `logic_basics/src/cpu_v2/`: zweite CPU
- `logic_basics/test/`: C++-Testbenches und Softwaremodelle
- `logic_basics/docs/`: Lernkapitel

Der Lernweg beginnt mit der [Ampel-FSM](logic_basics/docs/fsm_ampel.md)
und führt über die [ALU](logic_basics/docs/alu4.md) und
[CPU v1](logic_basics/docs/akkumulator4.md) zum
[ISA-Entwurf von CPU v2](logic_basics/docs/cpu_v2/isa.md).
Die Kapitel sind untereinander verlinkt.

[Architektur-Spickblatt: Bausteine und Signalfluss](logic_basics/docs/architektur.md).

## Editor und Tools

- [Spade-LSP in VS Code](logic_basics/docs/vscode_lsp.md)
- [Spade-/Swim-Migration nach Codeberg](logic_basics/docs/spade_migration.md)
- [Surfer](https://surfer-project.org/) für VCD-Signalverläufe

Weitere Informationen: [Roadmap](ROADMAP.md) ·
[AGENTS.md](AGENTS.md) · [Spade-Dokumentation](https://docs.spade-lang.org/).
