# Agent Guidance

## Projekt

HDL_Stuff ist eine Sammlung kleiner Experimente zu digitalen Schaltungen, HDLs und FPGA-Entwicklung. Aktuell wird Spade mit Swim und Verilator verwendet.

## Arbeitsweise

- Verständnis vor Implementierung: zuerst Schaltungsverhalten, Wahrheitstabellen und Hardwarestruktur klären, dann HDL-Code.
- Lernaufteilung: Der Nutzer implementiert die Spade-Hardware selbst; der Agent pflegt möglichst testgetrieben Wahrheitstabellen, C++-Testbenches, Makefile-Targets, VCD-Tracing und Editor-Konfiguration.
- Spade-Lösungen nicht vorwegnehmen. Gezielt erklären und Rückfragen zur Hardware stellen; vollständigen Spade-Code nur auf Wunsch oder zur Fehlerbehebung geben.
- Spade-Beispiele an der gepinnten Compiler-Version ausrichten. Für neue `inst`-Aufrufe benannte Ports mit geschweiften Klammern bevorzugen: `inst modul::entity { portname: signal, ... }`. Der gepinnte Parser unterstützt auch Positionsargumente `(...)` und benannte Argumente `$(...)`, aber keine benannten Argumente als `inst modul::entity(portname: signal)`. Bestehende ältere Module nicht allein für den Stilwechsel umbauen.
- CPU v1 unter `logic_basics/src/cpu/` als funktionsfähige Referenz erhalten. Die zweite Generation mit 8-Bit-Befehlen entsteht getrennt unter `logic_basics/src/cpu_v2/`; ihr ISA-Entwurf liegt in `logic_basics/docs/cpu_v2/isa.md`. Nicht vorzeitig v1-Decoder, ROM, CPU-Top oder Tests überschreiben. Neue Spade-Module erst zur jeweiligen Übung anlegen und dann im Hauptmodul registrieren.
- Quellcode übersichtlich halten und Kommentare auf Deutsch schreiben. Kommentare sollen Entscheidungen und nicht offensichtliche Zusammenhänge erklären, keine trivialen Zeilen wiederholen.
- Simulation, Verifikation, Synthese und Implementierung klar unterscheiden.
- Kleine, nachvollziehbare Änderungen bevorzugen. Keine unnötigen Abstraktionen oder Frameworks.
- Vor Änderungen Repository und Dateistand prüfen; keine nicht gepushten lokalen Änderungen voraussetzen oder überschreiben.
- Öffentliche Dokumentation kurz, technisch und ohne Chatprotokolle halten.
- Die Haupt-README knapp als Einstieg pflegen: Setup, wichtigste Make-Targets, Projektstand und Links. Umfangreiche Installationshinweise, vollständige Testlisten und Historien in eigene Dokumente oder `make help` auslagern. Sachlich und ohne übertriebenes Statusmarketing formulieren.
- Linux und macOS berücksichtigen. Nix-Dev-Shell verwenden; keine festen Nix-Store-Pfade einchecken.
- VS-Code-Workspace `HDL_Stuff.code-workspace` kombiniert LSP mit Projektwurzel `logic_basics/` und Make-Tasks im Repository-Root. Bei Änderungen an `.vscode/tasks.json` die Aufgaben in dieser Workspace-Datei synchron halten. Die eventuell lokale `logic_basics/.vscode/` nicht ungefragt ändern.
- Bestehende Make-Targets und Testabläufe nach Änderungen möglichst erhalten.
- C++-Hardwaretests nach `logic_basics/test/foundations/`, `test/cpu/` und `test/cpu_v2/` sortieren. Neue Testgruppen im Makefile zuordnen. Die Theorie folgt derselben Gruppierung; LSP- und Compiler-Notizen liegen unter `logic_basics/docs/tooling/`.
- Der C++17-Mini-Assembler ist ein separates Lernprojekt unter `logic_basics/assembler/`, seine Einführung unter `logic_basics/docs/assembler/`. Die eigentliche Software-Implementierung schreibt der Nutzer; Tests und Gerüst können vorbereitet werden. Kapitel II unter `logic_basics/docs/assembler/02_parser.md`; der offene `parse_line`-Platzhalter soll weder Parser- noch Encoder-Lösung vorwegnehmen. Maximal 16 ROM-Wörter und 4-Bit-Operanden; ungültige Eingaben müssen als Fehler enden. Eine offene Softwareübung nicht stillschweigend in `make test_completed` aufnehmen.
- Bei neuen Tests die Makefile-Zuordnung `TEST_NAMES`, `TEST_TOP_<name>`, `TEST_SRCS_<name>` und die C++-Quellgruppe ergänzen. Nur tatsächliche Spade-Abhängigkeiten aufnehmen, damit unabhängige Verilator-Tests nicht unnötig neu kompiliert werden. `make test` soll weiterhin alle Tests ausführen; `make clean` entfernt den generierten Build und die VCD-Dateien.
- Das Makefile selbst nicht pauschal als Build-Abhängigkeit eintragen: Änderungen an Targets, Kommentaren oder der Liste abgeschlossener Übungen dürfen vorhandene Test-Binaries nicht invalidieren. Bei Änderungen von Compiler-Flags oder Build-Kommandos muss bewusst `make clean` ausgeführt werden; Toolchain-Dateien bleiben echte Abhängigkeiten.

## Lernkapitel für neue Übungen

- Für jede neue Hardware-Übung standardmäßig ein Theorie- und Spade-Kapitel unter `logic_basics/docs/foundations/`, `docs/cpu/` oder `docs/cpu_v2/` vorbereiten. Zuvor den tatsächlichen Stand der Schaltung, angrenzender Module, Tests und bestehenden Notizen lesen. Neue Kapitel im `README.md` verlinken, wenn die Übung in das Projekt aufgenommen wird.
- Die Kapitel sollen aufeinander aufbauen und die nächste Hardwareaufgabe verständlich machen, ohne die fertige Implementierung zu verraten.
- Lernkapitel am Dateiende mit einer knappen Vor-/Zurück-Navigation verbinden (relative Markdown-Links zum tatsächlich vorherigen und folgenden Kapitel). Bei erstem und letztem Kapitel stattdessen zur README-Übersicht verlinken. Technische Nachschlageseiten wie LSP-Setup gehören nicht künstlich in diese Lernfolge.
- Bewährter Kapitelaufbau: Lernziel und CPU-Einordnung; Verhalten und Signalprioritäten; Hardwarestruktur (bei Bedarf ASCII-Blockdiagramm); Zustandsübergänge mit Tabellen oder Formeln; notwendige Spade-Sprachmittel an kleinen unabhängigen Beispielen; klare Implementierungsaufgabe mit Datei/Ports; Testfälle, Make-Target und VCD-Pfad; kurzer Ausblick.
- Insbesondere bei Registern, FSMs und CPU-Bausteinen den aktuellen und nächsten Zustand sauber unterscheiden. Bedingungen, Flanken, Reset-Priorität sowie synchrones versus kombinatorisches Verhalten ausdrücklich erklären.
- Konkrete Wahrheitstabellen, Zustandsformeln und kleine Beispiele bevorzugen, statt nur APIs oder Syntax aufzuzählen. Mathematische Fallunterscheidungen dürfen auch in Markdown mit LaTeX dargestellt werden.
- Bei neuen Aufgaben nach Möglichkeit einen kompilierbaren, aber absichtlich unvollständigen Spade-Platzhalter und unabhängige Tests bereitstellen. Tests sollen das zugesagte Verhalten prüfen; bestehende Lösungen nicht verändern oder vorwegnehmen.
- Für Aufgaben mit mehreren Bausteinen die konkrete Aufgabe zu Beginn in wenigen Sätzen eingrenzen: Welche Zustände oder Ausgänge werden implementiert, welche Signale sind bereits berechnet, und welche Nachbarbausteine bleiben ausdrücklich außen vor? Erst danach Portliste und Detailtabellen zeigen.
- Bei Registerübungen zuerst die getrennten Entscheidungen pro Register erklären (Schreibfreigabe, Datenquelle, Sprungbedingung), danach die gemeinsamen Prioritäten Reset/Pause und schließlich die HDL-Umsetzung. Einen normalen Ablauf mit alten und neuen Werten durchrechnen.
- Neue Spade-Sprachmittel, die für die Lösung zwingend sind (z. B. `decl` für Vorwärtsreferenzen bei gegenseitig abhängigen Registern), vor der eigentlichen Aufgabe mit einem isolierten Mini-Beispiel einführen. Dabei immer erklären, ob Syntax neue Hardware erzeugt oder nur die Quellcodeorganisation betrifft.
- Gewöhnliche Steuerkombinationen der ISA und zusätzliche, künstliche Testkombinationen ausdrücklich unterscheiden. Ungewöhnliche Testfälle begründen und mindestens ein Beispiel mit erwarteten Registerwerten angeben, damit der Testvertrag vor der Implementierung verständlich ist.
- Den TDD-Platzhalter nicht so kommentieren, als sei seine unvollständige Ausgangslogik das Ziel. Bei Abschluss veraltete TODOs und Stub-Hinweise entfernen. Öffentliche Lernkapitel als allgemeine technische Dokumentation verfassen: keine Gesprächsverläufe, persönlichen Lernschwierigkeiten oder impliziten Hinweise auf einen Chat.
- Nach gemeldeten bestandenen Tests zuerst den tatsächlich gepushten Code reviewen; bei Pflege nur Namen, Formatierung und erklärende Kommentare verändern, ohne die vom Lernenden implementierte Hardwarelogik zu ersetzen. Testaussagen unterscheiden: vom Nutzer gemeldet, selbst ausgeführt und nur statisch geprüft.
- Nach dem Abschluss einer Übung README, Kapitelstatus, `make test_completed` und VS-Code-Standardtask konsistent halten. Die nächste offene Aufgabe erst als abgeschlossen markieren, wenn dafür ein entsprechender Testlauf vorliegt.

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
