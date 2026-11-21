# Roadmap

Grobe Lern- und Projektreihenfolge, ohne feste Termine:

1. **Kombinatorische Logik:** Gatter, Multiplexer, Addierer; Simulation und Testbenches.
2. **Sequentielle Logik:** Flipflops, Register, Zähler und Schieberegister.
3. **Steuerlogik:** Zustandsautomaten und einfache Schnittstellen.
4. **Datenpfade:** Kleine ALU und zusammengesetzte Module.
5. **Computerarchitektur:** 4-Bit-CPU mit Decoder, Ausführungsstufe, Programmzähler und ROM.
6. **Softwaremodell:** Unabhängiger CPU-Emulator zum Funktionstest; später optional GUI und Debugger.

## Weitere Etappen

- **CPU v2 abschließen:** Die bereits getesteten Bausteine im gemeinsamen Top-Level verbinden und gegen das ISA-Softwaremodell prüfen.
- **Kleiner Assembler (danach):** Als eigene angeleitete Programmierübung lesbare Befehle und optional Sprunglabels in einen Spade-`match address`-Block für ein ROM übersetzen. Rust oder C++ erst beim Entwurf entscheiden; keine fertige Implementierung vorwegnehmen.
- **CPU v3 (später):** 8-Bit-Datenpfad und RAM ausprobieren. Befehlssatz, Befehlsbreite, Adressierung und Speicherarchitektur erst vor dem Bau gemeinsam festlegen.

Querschnittsthemen: Waveforms, Syntheseergebnisse, Timing, Debugging und nachvollziehbare Tests.

Später optional: weitere Hardwarebeschreibungstechniken wie [Hardcaml](https://github.com/janestreet/hardcaml), sobald die Spade-Grundlagen sitzen.
