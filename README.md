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

### Spade-Umzug nach Codeberg (bestehendes Projekt)

Spade ist am 18. September 2026 von GitLab nach
[Codeberg](https://codeberg.org/spade-lang/) umgezogen. Ältere Projekte
können deshalb beim Bauen `Spade has moved. Update your project` melden.

**Wichtig bei Nix:** Der hier gepinnte nixpkgs-Stand enthält Spade
und Swim **0.20.0**, eine Version vom 20. August 2026, also *vor* dem
Umzug. Die Paketquellen kommen bereits von Codeberg; daraus folgt aber
nicht, dass dieses Swim-Release die Migration älterer Projekte unterstützt.
`nix flake update` allein ist daher keine garantierte Lösung.

Zuerst den tatsächlich verwendeten Compiler-Pin prüfen:

```sh
nix develop
swim --version
cat logic_basics/swim.lock
git status --short
```

Für die einmalige Migration verlangt die
[offizielle Anleitung](https://blog.spade-lang.org/moving-to-codeberg/)
eine Swim-Version **nach dem Umzug** und danach diese Reihenfolge im
Verzeichnis `logic_basics/`:

```sh
swim clean --and-spade
swim update-spade
swim build
```

Wenn danach weiterhin der alte Commit `177e5c46` in `swim.lock` steht,
hat die Migration **nicht** stattgefunden. Erst eine neuere Swim-Version
verwenden, die den Codeberg-Umzug unterstützt, beispielsweise wie in
der offiziellen Anleitung aus dem aktuellen Codeberg-Repository installiert.
Wer alle Werkzeuge ausschließlich über Nix verwalten möchte, kann
stattdessen den Swim-Paketstand in der Nix-Dev-Shell gezielt auf eine
Version nach dem Umzug anheben. Die bestehende Lockdatei nicht von Hand
auf einen geratenen Commit ändern.

Nach erfolgreichem Update mit `make test` prüfen und erst dann die
geänderte `logic_basics/swim.lock` committen.

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
make test              # Alle derzeitigen Tests
make clean             # Build-Artefakte und VCD-Traces löschen
make test_completed    # Nur bereits abgeschlossene Übungen prüfen
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
make test_alu4          # Nur die 4-Bit-ALU testen
make test_alu4_ripple   # Ripple-Carry-Vergleichsvariante prüfen
make test_accumulator4  # 4-Bit-Akkumulator testen
make test_decoder6      # 6-Bit-Decoder testen
make test_instruction_exec6 # Befehle in Hardware und Emulator vergleichen
make test_pc4          # 4-Bit-Programmzähler testen
make test_rom16x6       # 16x6-Bit-Programmspeicher testen
make synth_alu4         # ALU mit Yosys analysieren (optionaler Einstieg)
make compare_alu4       # Beide ALUs mit Yosys vergleichen
```

Jede Testbench schreibt eine VCD-Datei nach `logic_basics/build/<testname>.vcd`
(z. B. `decoder6.vcd`). Das Verzeichnis `build/` wird nicht eingecheckt.

Die Test-Binaries werden inkrementell gebaut: `make test` führt jedes Testprogramm
bei jedem Aufruf aus, kompiliert aber nur neu, wenn passende Spade-Quellen,
C++-Tests oder Toolchain-Eingaben verändert wurden. Eine Änderung an
`pc4.spade` löst daher keinen Neubau der unabhängigen ALU-Tests aus.
Änderungen an der Testverwaltung im Makefile lösen keinen Neubau aus.
Wenn sich hingegen Compiler-Flags oder die Verilator-/Swim-Build-Kommandos
ändern, ist `make clean` bewusst erforderlich. Geänderte Toolchain-Dateien
wie `flake.lock` oder `swim.lock` werden weiterhin automatisch berücksichtigt.
`make clean` entfernt das komplette `logic_basics/build/` inklusive
VCD-Dateien; der nächste Testlauf baut anschließend alles frisch.
`make help` listet alle verfügbaren Targets.

**Stand:** Akkumulator, Decoder, Befehlsausführungsstufe, 4-Bit-Programmzähler und kombinatorischer **16x6-Bit-ROM** sind implementiert. Für den ROM wurden **608/608 Prüfungen** als bestanden gemeldet. Der Test ist nun auch in `make test_completed` enthalten. Die nächste Übung wird die Verbindung von PC, ROM und Ausführungsstufe behandeln.

**Synthese-Einstieg:** `make synth_alu4` startet Yosys nach dem Swim-Build und schreibt einen Bericht nach `logic_basics/build/alu4_synth.log`. Die Zahlen beschreiben eine generische, optimierte Gatternetzliste, noch keine konkrete FPGA-Ressourcenbelegung. Mit `make compare_alu4` werden beide ALU-Varianten unter identischen Syntheseschritten einschließlich `synth -flatten` verglichen. Neben den Zellzahlen zeigt `ltp -noff` die längsten Logikpfade als Stufenzahl an. Das sind keine physikalischen Laufzeiten. Näheres steht in den [ALU-Lernnotizen](logic_basics/docs/alu4.md).

### Quellcode-Struktur

```text
logic_basics/
├── src/
│   ├── main.spade                 # Beide Modulgruppen einbinden
│   ├── foundations/
│   │   ├── main.spade             # Gatter, Addierer, Register, FSM und Vergleichs-ALU
│   │   ├── alu_ripple.spade       # Selbstgebauter Ripple-Carry-Addierer in ALU
│   │   └── *.spade
│   └── cpu/
│       ├── main.spade             # ALU, Akkumulator, Decoder, Ausführungsstufe
│       ├── alu.spade
│       ├── accumulator.spade
│       ├── decoder6.spade
│       ├── instruction_exec6.spade
│       ├── pc4.spade
│       └── rom16x6.spade
├── test/                          # Verilator-C++-Tests und Software-Referenzmodell
├── docs/                          # Lernnotizen und Spezifikationen
└── scripts/                       # Auswertung der Yosys-Logs
```

Die internen Spade-Importe nutzen `lib::foundations::...` bzw.
`lib::cpu::...`. Die exportierten Verilator-Top-Namen bleiben gleich.

### VS-Code-Testtask

Im Repository-Root als VS-Code-Workspace: `Cmd+Shift+P` (macOS) bzw.
`Ctrl+Shift+P` (Linux), dann **Tasks: Run Test Task** ausführen.
Der Standard-Testtask heißt **HDL: Aktuelle Übung (16x6-Bit-ROM)**
und führt `nix develop --command make test_rom16x6` aus.

Unter **Tasks: Run Task** gibt es außerdem **HDL: Einzeltest auswählen**
(mit Auswahl aller vorhandenen Make-Testtargets) und **HDL: Alle Tests (Nix)**
für den vollständigen Regressionstest. Den Standard-Testtask passen wir
bei neuen Übungen an. Die Nix-CLI muss aus VS Code erreichbar sein.

Die Begleitnotizen zur Ampel stehen unter [`logic_basics/docs/fsm_ampel.md`](logic_basics/docs/fsm_ampel.md).
Die ALU-Spezifikation steht unter [`logic_basics/docs/alu4.md`](logic_basics/docs/alu4.md).
Die Akkumulator-Notizen stehen unter [`logic_basics/docs/akkumulator4.md`](logic_basics/docs/akkumulator4.md).
Der Einstieg in Maschinenbefehle und Decoder steht unter [`logic_basics/docs/decoder6.md`](logic_basics/docs/decoder6.md).
Die Befehlsausführungsstufe und das erste Emulator-Referenzmodell stehen unter [`logic_basics/docs/instruction_exec6.md`](logic_basics/docs/instruction_exec6.md).
Der Einstieg in den Programmzähler steht unter [`logic_basics/docs/pc4.md`](logic_basics/docs/pc4.md).
Die ROM-Theorie und abgeschlossene Spade-Übung stehen unter [`logic_basics/docs/rom16x6.md`](logic_basics/docs/rom16x6.md).

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
