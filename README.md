# HDL_Stuff

HDL- und FPGA-Experimente mit Fokus auf [Spade](https://docs.spade-lang.org/).

## Setup

Entwicklungsumgebung für Linux und macOS mit [Nix Flakes](https://nixos.wiki/wiki/Flakes).

```sh
git clone https://github.com/Nikoshi/HDL_Stuff.git
cd HDL_Stuff
nix develop
```

Die Dev-Shell enthält Spade, Swim, Verilator, Yosys und GNU Make. Unter Linux ist außerdem Surfer enthalten.

Falls Flakes noch nicht aktiviert sind:

```sh
nix --extra-experimental-features 'nix-command flakes' develop
```

### Waveform-Viewer

[Surfer](https://surfer-project.org/) und [GTKWave](https://gtkwave.sourceforge.net/) visualisieren Signalverläufe aus Simulationen, unter anderem im VCD- und FST-Format. GTKWave bietet eine klassische Oberfläche mit etablierten Debugging-Funktionen; Surfer setzt auf eine modernere Oberfläche und Erweiterbarkeit.

- **Linux:** Surfer ist in der Nix-Dev-Shell enthalten.
- **macOS:** Die erzeugten VCD-Dateien lassen sich in der [Surfer-Webapp](https://app.surfer-project.org/) öffnen. Alternativ kann GTKWave über [MacPorts](https://ports.macports.org/port/gtkwave/) installiert werden, sofern es mit der verwendeten macOS-Version funktioniert:

```sh
sudo port install gtkwave
```

Surfer lässt sich bei Bedarf auch unter macOS [aus dem Quellcode installieren](https://docs.surfer-project.org/book/). Beide Viewer können dieselben üblichen Simulationsdateien öffnen.

## Build und Tests

Im Repository-Root innerhalb der Nix-Dev-Shell:

```sh
make logic_basics      # Spade-Code übersetzen
make test              # Alle Testbenches ausführen
make test_mux          # Nur den Multiplexer testen
make test_half_adder   # Nur den Halbaddierer testen
make test_full_adder   # Nur den Volladdierer testen
make test_add2         # Nur den 2-Bit-Addierer testen
make test_add4         # Nur den 4-Bit-Addierer testen
make test_register1    # Nur das 1-Bit-Register testen
make test_register4    # Nur das 4-Bit-Register mit Enable testen
make test_counter4     # Nur den 4-Bit-Zähler mit Enable testen
make test_shift4       # Nur das 4-Bit-Schieberegister testen
make test_traffic_light # Nur die Ampel-FSM testen
```

Die Tests erzeugen `logic_basics/build/mux.vcd`, `logic_basics/build/half_adder.vcd`, `logic_basics/build/full_adder.vcd`, `logic_basics/build/add2.vcd`, `logic_basics/build/add4.vcd` und `logic_basics/build/register1.vcd`, `logic_basics/build/register4.vcd` sowie `logic_basics/build/counter4.vcd` und `logic_basics/build/shift4.vcd` und `logic_basics/build/traffic_light.vcd`. Das Verzeichnis `build/` wird nicht eingecheckt. `make help` zeigt die verfügbaren Targets.

**TDD:** Die bisherigen Schaltungen sind implementiert. Die Ampel-FSM ist als bewusst unvollständiger Platzhalter angelegt; `make test_traffic_light` und zunächst auch `make test` sollen daher fehlschlagen.

### Quellcode-Struktur

- `logic_basics/src/main.spade`: bindet Spade-Module mit `mod` ein.
- `logic_basics/src/mux.spade`: 2:1-Multiplexer.
- `logic_basics/src/adders.spade`: kombinatorische Addierer.
- `logic_basics/src/test_tops.spade`: Verilator-Schnittstellen der Addierer.
- `logic_basics/src/registers.spade`: Register mit synchronem Reset und Register-Enable.
- `logic_basics/src/counters.spade`: Aufwärtszähler mit festgelegter Bitbreite.
- `logic_basics/src/shift_registers.spade`: serielles Schieberegister mit Enable und Reset.
- `logic_basics/src/fsm.spade`: erste Moore-FSM mit vier Ampelzuständen.

### VS-Code-Testtask

Im Repository-Root als VS-Code-Workspace: `Cmd+Shift+P` (macOS) bzw.
`Ctrl+Shift+P` (Linux), dann **Tasks: Run Test Task** ausführen.
Der Standard-Testtask heißt derzeit **HDL: Aktuelle Übung (Ampel-FSM)**
und führt `nix develop --command make test_traffic_light` aus.

Unter **Tasks: Run Task** gibt es außerdem **HDL: Einzeltest auswählen**
(mit Auswahl aller vorhandenen Make-Testtargets) und **HDL: Alle Tests (Nix)**
für den vollständigen Regressionstest. Den Standard-Testtask passen wir
bei neuen Übungen an. Die Nix-CLI muss aus VS Code erreichbar sein.

Die Begleitnotizen zur Ampel stehen unter [`logic_basics/docs/fsm_ampel.md`](logic_basics/docs/fsm_ampel.md).

## Neues Projekt

```sh
swim init <projektname>
cd <projektname>
swim build
```

Swim verwaltet die Projektkonfiguration in `swim.toml` und übersetzt Spade nach SystemVerilog. Für Simulationen und Tests kann die Toolchain mit Verilator verwendet werden.

## Geplante Projekte

1. Logikgatter, Multiplexer und Addierer
2. Register, Zähler und Schieberegister
3. Zustandsautomaten und einfache Schnittstellen
4. Kleine ALU
5. 4-Bit-CPU

## Projektinfos

- [Roadmap](ROADMAP.md)
- [Agent Guidance](AGENTS.md)
- [Neuer Chat](NEW_CHAT.md)

## Links

- [Spade Documentation](https://docs.spade-lang.org/)
- [Swim](https://codeberg.org/spade-lang/swim)
- [Surfer](https://surfer-project.org/)
