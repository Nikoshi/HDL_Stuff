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

Wichtig: **`HDL_Stuff/` als VS-Code-Workspace öffnen**, nicht
nur `logic_basics/`. Nix sucht dort unsere `flake.nix`.

Die Spade-Projektkonfiguration befindet sich dagegen unter
`logic_basics/swim.toml`. Daher startet der LSP-Prozess erst
nach dem Wechsel in dieses Unterverzeichnis.

## 3. Hinterlegte Workspace-Einstellungen

Die bereits eingecheckte `.vscode/settings.json` enthält:

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

Das entspricht in einer Shell, aus dem Repository-Root:

```sh
nix develop -c sh -c 'cd logic_basics && exec swim lsp'
```

Die `spade-lsp.serverCommand`-/`serverArgs`-Felder sind die
dokumentierte Schnittstelle der Erweiterung. Mit `exec` bleibt
der LSP-Server der laufende Prozess, dessen Standard-Ein- und -Ausgabe
vom VS-Code-LSP-Client verwendet werden können.

Es ist kein globales `swim` und **keine VS-Code-Paketaufnahme
in `flake.nix`** erforderlich.

## 4. Inbetriebnahme und Fehlersuche

- Nach dem Pull die beiden Erweiterungen installieren und eine
  `.spade`-Datei öffnen. Die Erweiterung kann über die
  Befehlspalette mit **Spade: Restart Language Server** neu gestartet werden.
- Funktioniert ein Start nicht, im VS-Code-Ausgabefenster
  (**Output**, Kanal **Spade LSP**) nachsehen; bei Bedarf in
  `.vscode/settings.json` `"spade-lsp.trace.server": "verbose"`
  vorübergehend ergänzen.
- Kann VS Code den Befehl `nix` nicht finden, den Editor aus
  einem Terminal mit funktionierendem Nix-`PATH` starten,
  zum Beispiel aus dem Repository-Root über `code .`.
  Bei GUI-Starts kann sich die geerbte Umgebung unterscheiden.
- Die Projektversionen bleiben durch `flake.lock` und
  `logic_basics/swim.lock` festgelegt. Wenn `swim lsp` an
  einem alten Compiler-Pin oder an der Swim-Migration scheitert,
  die Ursache im Output prüfen; die Lockdatei nicht auf Verdacht ändern.

**Status:** Repository-Konfiguration vorbereitet, aber der Start
in deinem lokalen VS Code wurde noch nicht verifiziert.
