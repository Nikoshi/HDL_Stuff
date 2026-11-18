# Agent Guidance

## Projekt

HDL_Stuff ist eine Sammlung kleiner Experimente zu digitalen Schaltungen, HDLs und FPGA-Entwicklung. Aktuell wird Spade mit Swim und Verilator verwendet.

## Arbeitsweise

- Verständnis vor Implementierung: zuerst Schaltungsverhalten, Wahrheitstabellen und Hardwarestruktur klären, dann HDL-Code.
- Beim Lernen nur gezielte Hinweise, Beispiele und Erklärungen geben. Übungen und Testbench-Erweiterungen möglichst selbst implementieren lassen; vollständigen Code nur auf Wunsch liefern oder wenn die technische Hürde unverhältnismäßig ist.
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
- `logic_basics/`: Spade-Übungen; generierte Dateien in `build/` werden ignoriert.

Für den **aktuellen** Stand die Dateien im Repository lesen; diese Anleitung ist bewusst nicht an einen bestimmten Meilenstein gebunden.
