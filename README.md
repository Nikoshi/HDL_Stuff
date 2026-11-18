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

## Neues Projekt

```sh
swim init logic_basics
cd logic_basics
swim build
```

Swim verwaltet die Projektkonfiguration in `swim.toml` und übersetzt Spade nach SystemVerilog. Für Simulationen und Tests kann die Toolchain mit Verilator verwendet werden.

## Links

- [Spade Documentation](https://docs.spade-lang.org/)
- [Swim](https://codeberg.org/spade-lang/swim)
- [Surfer](https://surfer-project.org/)
