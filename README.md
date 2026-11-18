# HDL_Stuff

Experimente mit digitalen Schaltungen, HDLs und FPGA-Entwicklung. Wir starten mit [Spade](https://docs.spade-lang.org/) und kleinen, gut testbaren Bausteinen. Eine CPU kommt erst später. :)

## Quick Start (Nix: Linux & macOS)

Voraussetzung: [Nix](https://nixos.org/download/) mit aktivierten `nix-command`- und `flakes`-Features.

```sh
git clone https://github.com/Nikoshi/HDL_Stuff.git
cd HDL_Stuff
nix develop
```

Die [`flake.nix`](./flake.nix) stellt eine Entwicklungsumgebung mit **Spade**, **Swim**, **Verilator** und **Yosys** bereit. Unter Linux ist zusätzlich der Waveform-Viewer **Surfer** dabei. Es ist kein FPGA-Board erforderlich.

Installation kurz prüfen:

```sh
swim --help
spade --help
verilator --version
yosys -V
```

### Erstes Projekt erzeugen

Im Repository-Root, innerhalb der Nix-Entwicklungsumgebung:

```sh
swim init logic_basics
cd logic_basics
swim build
```

Swim legt ein eigenes Projekt mit `swim.toml` und Spade-Quelltext in `src/` an. `swim build` übersetzt den Spade-Code zu SystemVerilog (normalerweise `build/spade.sv`).

Sobald wir Testbenches hinzugefügt haben, kommt außerdem `swim test` bzw. `swim simulate` dazu. Für die ersten Schritte reicht es, das generierte Projekt zu bauen.

### Waveforms anzeigen

- **Linux:** `surfer <datei.fst>` (Surfer ist Teil der Dev-Shell).
- **macOS:** Surfer kann optional separat installiert werden, z. B. mit `brew install surfer`. Die Spade-/Swim-Toolchain selbst kommt auch dort aus Nix.

Waveform-Dateien entstehen erst, wenn entsprechende Simulationstests eingerichtet und ausgeführt werden.

### Nix-Hinweise

- Beim ersten `nix develop` lädt Nix die benötigten Pakete herunter.
- Falls Flakes nicht global aktiviert sind: `nix --extra-experimental-features 'nix-command flakes' develop`.
- Die beim ersten Aufruf erzeugte `flake.lock` sollte ins Git-Repo committed werden, damit beide Rechner dieselben nixpkgs-Versionen nutzen.
- Swim verwaltet pro Projekt auch dessen Spade-Compiler-Abhängigkeit. Eine initiale Projekt-Erstellung bzw. ein Build kann deshalb zusätzliche Quellen herunterladen.

## Geplanter Lernpfad

1. Kombinatorische Logik (Gatter, Multiplexer, Halbaddierer)
2. Sequentielle Logik (Flipflops, Register, Zähler)
3. Simulation und Testbenches
4. Zustandsautomaten und kleine Schnittstellen
5. Später: eine eigene 4-Bit-CPU

## Referenzen

- [Spade-Dokumentation](https://docs.spade-lang.org/)
- [Spade-Installation](https://docs.spade-lang.org/installation.html)
- [Swim-Projekte](https://docs.spade-lang.org/swim_project.html)
- [Simulation und Tests](https://docs.spade-lang.org/simulation.html)
- [Surfer](https://surfer-project.org/)
