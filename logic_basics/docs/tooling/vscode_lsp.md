# Spade-LSP in VS Code über Nix

VS Code und die beiden Erweiterungen werden **normal lokal installiert**,
nicht über die Nix-Flake. Nix liefert ausschließlich die im Repository
bereits verwendeten Spade-/Swim-Werkzeuge.

## 1. Erweiterungen installieren

Im VS-Code-Erweiterungsfenster (Strg+Umschalt+X bzw. Cmd+Umschalt+X):

1. **Spade Syntax** (`spade-lang.spade-syntax`) für Syntaxhervorhebung.
2. **Spade LSP** (`Mechazawa.vscode-spade-lsp`) für Diagnosen,
   Hover, Navigation und weitere vom Sprachserver unterstützte Funktionen.

Beide sind in `.vscode/extensions.json` empfohlen. Die offizielle
Spade-Syntaxerweiterung übernimmt nicht den hier separat benötigten
LSP-Client.

- [Spade Syntax im VS-Code-Marketplace](https://marketplace.visualstudio.com/items?itemName=spade-lang.spade-syntax)
- [Spade-LSP-Client und dessen Einstellungen](https://github.com/Mechazawa/vscode-spade-lsp)

## 2. Gemeinsamen Projekt-Workspace öffnen

Für LSP **und** Make-Tests ist im Repository-Root die Datei
[`HDL_Stuff.code-workspace`](../../../HDL_Stuff.code-workspace) hinterlegt.

In VS Code: **Datei → Arbeitsbereich aus Datei öffnen…**
(`File → Open Workspace from File…`) und diese Datei auswählen.
Alternativ im Repository-Root:

```sh
code HDL_Stuff.code-workspace
```

Dieser Workspace enthält `logic_basics/` als einzigen Projektordner.
Damit erkennt der Sprachserver die `swim.toml` und die Spade-Module
korrekt. Die Tasks laufen trotzdem im Repository-Root, wo das
`Makefile` und die `flake.nix` liegen.

Über **Terminal → Aufgabe ausführen…** kannst du wieder die bisherigen
Tests auswählen; **Testaufgabe ausführen** startet die Regression.
Der normale Terminalbefehl `make test` bleibt im Repository-Root
unverändert verfügbar.

## 3. Workspace-Einstellungen und Verzeichnisse

In `HDL_Stuff.code-workspace` stehen für den Spade-LSP:

```json
{
  "spade-lsp.serverCommand": "nix",
  "spade-lsp.serverArgs": [
    "develop", "..", "-c", "swim", "lsp"
  ]
}
```

Der Server wird aus `logic_basics/` gestartet und nutzt die Nix-Flake
im Elternverzeichnis. Die in derselben Workspace-Datei enthaltenen
Make-Tasks verwenden dagegen `"${workspaceFolder}/.."` als
Arbeitsverzeichnis. So haben beide Werkzeuge ihre richtige Projektwurzel,
ohne dass man zwischen VS-Code-Fenstern wechseln muss.

Die bisherige `.vscode/` im Repository-Root bleibt für das Öffnen
des gesamten Repositories bestehen. **Sie ist nicht die empfohlene
Variante für den Spade-LSP**, weil der Sprachserver dabei
möglicherweise den falschen Workspace-Root erhält.

Eine lokal angelegte `logic_basics/.vscode/settings.json` kann
Workspace-Einstellungen überschreiben. Falls der LSP im neuen
Workspace noch falsche Argumente verwendet, diese lokale Datei
vorübergehend umbenennen oder ihre LSP-Einträge entfernen;
nicht blind löschen. Der neue Workspace benötigt sie nicht.

## 4. Erststart und Fehlersuche

Der erste Aufruf von `swim lsp` kann auf macOS mangels passendem
vorgebautem Sprachserver einen längeren lokalen Cargo-Build auslösen.
Dabei können Build- und Statusmeldungen die LSP-Verbindung
stören (`Header must provide a Content-Length property`).
**Nach dem abgeschlossenen Build hat ein Neustart des LSP-Clients
in VS Code bereits erfolgreich funktioniert.** Das ist der im Projekt
beobachtete und für den Moment akzeptierte Startablauf.

- Nach einem `git pull` VS Code über **Developer: Reload Window**
  neu laden und eine `.spade`-Datei öffnen.
- Falls die initiale Verbindung während eines Cargo-Builds abbricht,
  den Build fertigstellen lassen und anschließend
  **Spade: Restart Language Server** ausführen.
- Im Ausgabefenster (**Output**, Kanal **Spade LSP**) stehen
  konkrete Fehlermeldungen. Optional kann vorübergehend
  `"spade-lsp.trace.server": "verbose"` aktiviert werden.
- Wenn VS Code `nix` nicht findet, den Editor aus einem Terminal
  mit eingerichtetem Nix-`PATH` starten, z. B. mit `code .`.
- `make clean` entfernt `logic_basics/build/` und damit auch
  das lokal erstellte Sprachserver-Binary. Beim nächsten
  `swim lsp` wird es bei Bedarf erneut gebaut; danach
  kann ein weiterer LSP-Neustart erforderlich sein.

Der alte Spade-Pin zeigt zwar einen Migrationshinweis, aber
`swim build` und der lokale Sprachserver-Build waren erfolgreich.
Für die hier beobachtete Erststart-Störung ist eine
Compiler-Migration nicht erforderlich.

**Status:** Der Compiler-Build funktioniert und das separate Öffnen
von `logic_basics/` hat die falschen LSP-Modulfehler lokal behoben.
Der gemeinsame `.code-workspace`-Ansatz verbindet diese Projektwurzel
mit Make-Tasks; seine Funktion ist bis zum lokalen VS-Code-Test
noch nicht bestätigt.

---

[← Zurück zur Projektübersicht](../../../README.md)
