# Agent Guidance

## Projekt

HDL_Stuff ist eine Sammlung kleiner Experimente zu digitalen Schaltungen, HDLs und FPGA-Entwicklung. Aktuell wird Spade mit Swim und Verilator verwendet.

## Arbeitsweise

- Verständnis vor Implementierung: zuerst Schaltungsverhalten, Wahrheitstabellen und Hardwarestruktur klären, dann HDL-Code.
- Lernaufteilung: Der Nutzer implementiert die Spade-Hardware selbst; der Agent pflegt möglichst testgetrieben Wahrheitstabellen, C++-Testbenches, Makefile-Targets, VCD-Tracing und Editor-Konfiguration.
- Spade-Lösungen nicht vorwegnehmen. Gezielt erklären und Rückfragen zur Hardware stellen; vollständigen Spade-Code nur auf Wunsch oder zur Fehlerbehebung geben.
- Spade-Beispiele an der gepinnten Compiler-Version ausrichten. Für `inst` funktionieren hier Positionsargumente; die Syntax neuerer Releases nicht ungeprüft übernehmen.
- Quellcode übersichtlich halten und Kommentare auf Deutsch schreiben. Kommentare sollen Entscheidungen und nicht offensichtliche Zusammenhänge erklären, keine trivialen Zeilen wiederholen.
- Simulation, Verifikation, Synthese und Implementierung klar unterscheiden.
- Kleine, nachvollziehbare Änderungen bevorzugen. Keine unnötigen Abstraktionen oder Frameworks.
- Vor Änderungen Repository und Dateistand prüfen; keine nicht gepushten lokalen Änderungen voraussetzen oder überschreiben.
- Öffentliche Dokumentation kurz, technisch und ohne Chatprotokolle halten.
- Linux und macOS berücksichtigen. Nix-Dev-Shell verwenden; keine festen Nix-Store-Pfade einchecken.
- Bestehende Make-Targets und Testabläufe nach Änderungen möglichst erhalten.

## Orientierung

- `README.md`: Einrichtung und Build-Befehle.
- `ROADMAP.md`: grobe Themen- und Projektreihenfolge, kein Fortschrittsprotokoll.
- `NEW_CHAT.md`: Einstieg für einen neuen Chat.
- `Makefile`: zentrale Build- und Test-Targets.
- `logic_basics/src/foundations/`: Logikgrundlagen; `logic_basics/src/cpu/`: CPU-Bausteine. Beide Gruppen haben eine `main.spade`.
- `logic_basics/`: generierte Dateien unter `build/` werden ignoriert.

Für den **aktuellen** Stand die Dateien im Repository lesen; diese Anleitung ist bewusst nicht an einen bestimmten Meilenstein gebunden.
