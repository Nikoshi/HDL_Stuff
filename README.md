# HDL_Stuff

HDL- und FPGA-Experimente mit Fokus auf [Spade](https://docs.spade-lang.org/).

## Setup

Entwicklungsumgebung für Linux und macOS mit [Nix Flakes](https://nixos.wiki/wiki/Flakes).

```sh
git clone https://github.com/Nikoshi/HDL_Stuff.git
cd HDL_Stuff
nix develop
```

Die Dev-Shell enthält Spade, Swim, Verilator und Yosys. Unter Linux ist außerdem Surfer enthalten.

Falls Flakes noch nicht aktiviert sind:

```sh
nix --extra-experimental-features 'nix-command flakes' develop
```

### Waveform-Viewer

[Surfer](https://surfer-project.org/) und [GTKWave](https://gtkwave.sourceforge.net/) visualisieren Signalverläufe aus Simulationen, unter anderem im VCD- und FST-Format. GTKWave bietet eine klassische Oberfläche mit etablierten Debugging-Funktionen; Surfer setzt auf eine modernere Oberfläche und Erweiterbarkeit.

- **Linux:** Surfer ist in der Nix-Dev-Shell enthalten.
- **macOS:** GTKWave kann über [MacPorts](https://ports.macports.org/port/gtkwave/) installiert werden:

```sh
sudo port install gtkwave
```

Surfer lässt sich bei Bedarf auch unter macOS [aus dem Quellcode installieren](https://docs.surfer-project.org/book/). Beide Viewer können dieselben üblichen Simulationsdateien öffnen.

## Neues Projekt

```sh
swim init logic_basics
cd logic_basics
swim build
```

Swim verwaltet die Projektkonfiguration in `swim.toml` und übersetzt Spade nach SystemVerilog. Für Simulationen und Tests kann die Toolchain mit Verilator verwendet werden.

## Geplante Projekte

1. Logikgatter, Multiplexer und Addierer
2. Register, Zähler und Schieberegister
3. Zustandsautomaten und einfache Schnittstellen
4. Kleine ALU
5. 4-Bit-CPU

## Links

- [Spade Documentation](https://docs.spade-lang.org/)
- [Swim](https://codeberg.org/spade-lang/swim)
- [Surfer](https://surfer-project.org/)
