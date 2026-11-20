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
- Bei neuen Tests die Makefile-Zuordnung `TEST_NAMES`, `TEST_TOP_<name>` und `TEST_SRCS_<name>` ergänzen. Nur tatsächliche Spade-Abhängigkeiten aufnehmen, damit unabhängige Verilator-Tests nicht unnötig neu kompiliert werden. `make test` soll weiterhin alle Tests ausführen; `make clean` entfernt den generierten Build und die VCD-Dateien.
- Das Makefile selbst nicht pauschal als Build-Abhängigkeit eintragen: Änderungen an Targets, Kommentaren oder der Liste abgeschlossener Übungen dürfen vorhandene Test-Binaries nicht invalidieren. Bei Änderungen von Compiler-Flags oder Build-Kommandos muss bewusst `make clean` ausgeführt werden; Toolchain-Dateien bleiben echte Abhängigkeiten.

## Lernkapitel für neue Übungen

- Für jede neue Hardware-Übung standardmäßig ein zur Übung gehörendes Theorie- und Spade-Kapitel unter `logic_basics/docs/<thema>.md` vorbereiten. Zuvor den tatsächlichen Stand der Schaltung, angrenzender Module, Tests und bestehenden Notizen lesen. Neue Kapitel im `README.md` verlinken, wenn die Übung in das Projekt aufgenommen wird.
- Die Kapitel sollen aufeinander aufbauen und die nächste Hardwareaufgabe verständlich machen, ohne die fertige Implementierung zu verraten.
- Bewährter Kapitelaufbau: Lernziel und CPU-Einordnung; Verhalten und Signalprioritäten; Hardwarestruktur (bei Bedarf ASCII-Blockdiagramm); Zustandsübergänge mit Tabellen oder Formeln; notwendige Spade-Sprachmittel an kleinen unabhängigen Beispielen; klare Implementierungsaufgabe mit Datei/Ports; Testfälle, Make-Target und VCD-Pfad; kurzer Ausblick.
- Insbesondere bei Registern, FSMs und CPU-Bausteinen den aktuellen und nächsten Zustand sauber unterscheiden. Bedingungen, Flanken, Reset-Priorität sowie synchrones versus kombinatorisches Verhalten ausdrücklich erklären.
- Konkrete Wahrheitstabellen, Zustandsformeln und kleine Beispiele bevorzugen, statt nur APIs oder Syntax aufzuzählen. Mathematische Fallunterscheidungen dürfen auch in Markdown mit LaTeX dargestellt werden.
- Bei neuen Aufgaben nach Möglichkeit einen kompilierbaren, aber absichtlich unvollständigen Spade-Platzhalter und unabhängige Tests bereitstellen. Tests sollen das zugesagte Verhalten prüfen; bestehende Lösungen nicht verändern oder vorwegnehmen.

## Feedback beim gemeinsamen Lernen

- Bei vorgelegtem Spade-Code zuerst das tatsächliche Verhalten nachvollziehen und dann **konkret** benennen, was daran korrekt ist. Ein Abschnitt wie „Was daran besonders gut ist“ ist sinnvoll, wenn sich darin eine überprüfbare Entwurfsentscheidung erklären lässt; kein pauschales Lob.
- Die zentrale Idee nach Möglichkeit als Zustandsformel oder kleine Fallunterscheidung zeigen (z. B. `ACC_neu = ALU(ACC, b, op)` bei Enable, sonst `ACC`). Erklären, wie diese Idee mit `reg`, Enable, Reset und Taktflanken zusammenhängt. Im Chat darf diese Darstellung ausführlicher und anschaulicher sein als die technische Projektdokumentation.
- Bei Fehlern die betroffene Bedingung, den erwarteten Zustand und die Ursache präzise erläutern. Zuerst einen gezielten Hinweis oder eine überschaubare Denkaufgabe geben; den vollständigen korrigierten Spade-Code nur auf ausdrücklichen Wunsch oder wenn er zur Fehlerbehebung gebraucht wird.
- Bei Feedback zwischen bewiesener Korrektheit, plausibler Überlegung und noch ungetestetem Verhalten unterscheiden. Test- und Syntheseergebnisse nur als bestanden ausgeben, wenn sie tatsächlich vorliegen.
- Wiederholungen vermeiden: bekannte Grundlagen nur dann erneut erklären, wenn sie für einen neuen Zusammenhang erforderlich sind.

## Orientierung

- `README.md`: Einrichtung und Build-Befehle.
- `ROADMAP.md`: grobe Themen- und Projektreihenfolge, kein Fortschrittsprotokoll.
- `NEW_CHAT.md`: Einstieg für einen neuen Chat.
- `Makefile`: zentrale Build- und Test-Targets.
- `logic_basics/src/foundations/`: Logikgrundlagen; `logic_basics/src/cpu/`: CPU-Bausteine. Beide Gruppen haben eine `main.spade`.
- `logic_basics/`: generierte Dateien unter `build/` werden ignoriert.

Für den **aktuellen** Stand die Dateien im Repository lesen; diese Anleitung ist bewusst nicht an einen bestimmten Meilenstein gebunden.
