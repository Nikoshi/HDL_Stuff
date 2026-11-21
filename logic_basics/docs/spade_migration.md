# Spade-/Swim-Umzug nach Codeberg

Spade ist am 18. September 2026 von GitLab zu
[Codeberg](https://codeberg.org/spade-lang/) umgezogen.
Bei älteren Projekten kann deshalb die Meldung
`Spade has moved. Update your project` erscheinen.

Unser gepinnter nixpkgs-Stand enthält noch Spade und Swim 0.20.0
vom August 2026. Die Paketquellen liegen zwar auf Codeberg,
das allein garantiert jedoch keine Unterstützung für die Migration.
Auch `nix flake update` behebt das nicht zwangsläufig.

## Version und Projektstand prüfen

Im Repository-Root:

```sh
nix develop
swim --version
cat logic_basics/swim.lock
git status --short
```

## Compiler-Pin migrieren

Für die Migration wird eine **Swim-Version nach dem Umzug** benötigt.
Die Reihenfolge aus der
[offiziellen Anleitung](https://blog.spade-lang.org/moving-to-codeberg/):

```sh
cd logic_basics
swim clean --and-spade
swim update-spade
swim build
```

Bleibt danach der alte Commit `177e5c46` in `swim.lock` stehen,
ist die Migration nicht erfolgt. In diesem Fall zuerst eine
neuere Swim-Version bereitstellen, etwa über einen angepassten
Nix-Paketstand. Die Lockdatei nicht auf einen geratenen Commit setzen.

Nach der Umstellung im Repository-Root `make test` ausführen
und erst danach die geänderte `logic_basics/swim.lock` committen.

**Hinweis:** Der alte Compiler-Pin hat beim bisherigen Arbeiten
`swim build` und den lokalen LSP-Build nicht verhindert.
Der Migrationshinweis allein ist daher kein Beleg für einen
fehlgeschlagenen Build.
