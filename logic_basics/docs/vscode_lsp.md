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
nach dem Wechsel in dieses Unterverzeichnis. Der erste Aufruf von
`swim lsp` kann den Sprachserver lokal mit Cargo bauen; das erfolgt
bewusst einmalig im Terminal und **nicht** über den LSP-Datenkanal.

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
    "cd logic_basics && exec ./build/spade/target/release/spade-language-server"
  ]
}
```

Das entspricht in einer Shell, aus dem Repository-Root:

```sh
nix develop -c sh -c 'cd logic_basics && exec ./build/spade/target/release/spade-language-server'
```

**Einmalige Voraussetzung:** Der Sprachserver muss bereits gebaut sein.
Dafür im Terminal (nicht im VS-Code-LSP-Prozess) ausführen:

```sh
nix develop -c sh -c 'cd logic_basics && swim lsp < /dev/null'
```

Der Befehl kann nach der Erstellung der ausführbaren Datei wieder enden;
es müssen dabei keine LSP-Anfragen gesendet werden. Anschließend prüfen:

```sh
nix develop -c sh -c 'test -x logic_basics/build/spade/target/release/spade-language-server && echo "Spade-LSP-Binary vorhanden"'
```

Der direkte Start umgeht die Status- und Cargo-Ausgaben von `swim lsp`,
die andernfalls auf `stdout` den LSP-Nachrichtenstrom beschädigen
können (`Header must provide a Content-Length property`).
Mit `exec` wird die bereits gebaute Server-Binary zum laufenden
Prozess, dessen Standardein- und -ausgabe VS Code verwendet.
Die `spade-lsp.serverCommand`-/`serverArgs`-Felder sind die
dokumentierte Konfigurationsschnittstelle der Erweiterung.

Es ist kein globales `swim` und **keine VS-Code-Paketaufnahme
in `flake.nix`** erforderlich.

## 4. Inbetriebnahme und Fehlersuche

- Nach dem Pull die beiden Erweiterungen installieren und eine
  `.spade`-Datei öffnen. **Developer: Reload Window** lädt die
  geänderte Workspace-Konfiguration neu. Anschließend lässt sich
  der Server über die Befehlspalette mit **Spade: Restart Language Server**
  bei Bedarf neu starten.
- Funktioniert ein Start nicht, im VS-Code-Ausgabefenster
  (**Output**, Kanal **Spade LSP**) nachsehen; bei Bedarf in
  `.vscode/settings.json` `"spade-lsp.trace.server": "verbose"`
  vorübergehend ergänzen.
- Kann VS Code den Befehl `nix` nicht finden, den Editor aus
  einem Terminal mit funktionierendem Nix-`PATH` starten,
  zum Beispiel aus dem Repository-Root über `code .`.
  Bei GUI-Starts kann sich die geerbte Umgebung unterscheiden.
- Die Projektversionen bleiben durch `flake.lock` und
  `logic_basics/swim.lock` festgelegt. `make clean` entfernt
  `logic_basics/build/` und damit auch das lokal gebaute LSP-Binary:
  danach den einmaligen `swim lsp`-Build wiederholen.
- Der Compiler-Pin meldet derzeit den Spade-Codeberg-Umzug, aber
  `swim build` und das lokale Erstellen des LSP wurden bereits erfolgreich
  beobachtet. Daher ist eine sofortige Compiler-Migration zur Behebung
  dieses speziellen Protokollfehlers nicht erforderlich.

**Status:** Fehlerursache anhand des VS-Code-Logs eingegrenzt:
`swim lsp` hat auf macOS einen lokalen Cargo-Build gestartet, dessen
Statusausgaben den LSP-Stream gestört haben. Die Workspace-Konfiguration
verwendet deshalb das bereits kompilierte Server-Binary direkt.
Der erfolgreiche Start in VS Code mit dieser Änderung ist noch zu prüfen.
