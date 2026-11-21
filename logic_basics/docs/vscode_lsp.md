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

## 2. Repository als Workspace öffnen

Wichtig: **`HDL_Stuff/` als VS-Code-Workspace öffnen**, nicht nur
`logic_basics/`. Nix sucht im Repository-Root die `flake.nix`;
`logic_basics/swim.toml` ist dagegen die Spade-Projektkonfiguration.

## 3. Hinterlegte Workspace-Einstellungen

Die eingecheckte `.vscode/settings.json` enthält:

```json
{
  "spade-lsp.serverCommand": "nix",
  "spade-lsp.serverArgs": [
    "develop",
    "-c",
    "sh",
    "-c",
    "cd logic_basics && exec swim lsp"
  ]
}
```

Das entspricht im Repository-Root:

```sh
nix develop -c sh -c 'cd logic_basics && exec swim lsp'
```

`swim lsp` startet den Sprachserver und baut ihn bei Bedarf automatisch.
Es ist **kein fester Pfad** zu einem bereits gebauten Binary erforderlich.
Auch VS Code selbst wird nicht in `flake.nix` aufgenommen.

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

**Status:** Der Start über `swim lsp` mit anschließendem LSP-Neustart
wurde lokal vom Nutzer als funktionierend bestätigt.

---

[← Zurück zur Projektübersicht](../../README.md)
