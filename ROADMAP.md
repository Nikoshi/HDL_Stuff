# Roadmap

Grobe Lern- und Projektreihenfolge, ohne feste Termine:

1. **Kombinatorische Logik:** Gatter, Multiplexer, Addierer; Simulation und Testbenches.
2. **Sequentielle Logik:** Flipflops, Register, Zähler und Schieberegister.
3. **Steuerlogik:** Zustandsautomaten und einfache Schnittstellen.
4. **Datenpfade:** Kleine ALU und zusammengesetzte Module.
5. **Computerarchitektur:** 4-Bit-CPU mit Decoder, Ausführungsstufe, Programmzähler und ROM.
6. **Softwaremodell:** Unabhängiger CPU-Emulator zum Funktionstest; später optional GUI und Debugger.

## Weitere Etappen

- **CPU v2 (abgeschlossen):** 4-Bit-Datenpfad, 8-Bit-Befehle, bedingte Sprünge und integrierter Test gegen das ISA-Softwaremodell.
- **Mini-Assembler (in Arbeit):** C++17-Lernprojekt mit abgeschlossenem Encoder (131/131 gemeldete Tests), abgeschlossenem Zeilenparser (38/38 gemeldete Tests), danach Sprunglabels und Spade-`match address`-Ausgabe. Das 16×8-Bit-ROM begrenzt Programme auf 16 Befehle; längere Eingaben sind Fehler. Hardware-Regression und offene Softwaretests bleiben getrennt.
- **CPU v3 (später):** 8-Bit-Datenpfad und RAM ausprobieren. Befehlssatz, Befehlsbreite, Adressierung und Speicherarchitektur erst vor dem Bau gemeinsam festlegen.

Querschnittsthemen: Waveforms, Syntheseergebnisse, Timing, Debugging und nachvollziehbare Tests.

Später optional: weitere Hardwarebeschreibungstechniken wie [Hardcaml](https://github.com/janestreet/hardcaml), sobald die Spade-Grundlagen sitzen.
