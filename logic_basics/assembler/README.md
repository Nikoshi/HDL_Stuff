# Mini-Assembler für CPU v2

Eigenständige C++17-Übung unterhalb von `logic_basics/`.
Die Spade-Hardware und die bestehende Hardware-Regression bleiben unverändert.

## Einstieg

1. **Abgeschlossen:** [Encoder](../docs/assembler/01_encoder.md), 131/131 gemeldete Tests.
2. **Abgeschlossen:** [Kapitel II: Zeilenparser](../docs/assembler/02_parser.md),
   38/38 gemeldete Tests; [C++17-Textverarbeitung](../docs/assembler/cpp17_text.md).
3. **Aktuell:** [Kapitel III: erster Programmdurchlauf](../docs/assembler/03_pass1.md),
   mit [C++17-Streams und Collections](../docs/assembler/cpp17_collections.md).
   Datei `pass1.cpp` implementieren. Labeladressen werden gesammelt,
   Sprungziele erst in einer späteren Etappe aufgelöst.

Im Repository-Root (Nix-Dev-Shell):

```sh
make test_assembler_encoder    # Abgeschlossener Encoder-Test
make test_assembler_parser     # Abgeschlossener Parser-Test
make test_assembler_pass1      # Offene Programmdurchlauf-Übung (rot)
make assembler                 # Noch unvollständige CLI bauen
make test_completed            # Bestehende Hardware-Regression
```

`test/encoder_test.cpp` prüft 131 Kodierfälle, `test/parser_test.cpp`
prüft 38 Zeilenfälle und `test/pass1_test.cpp` prüft 32 ganze
Programme. Das Executable `logic_basics/build/mini_assembler`
ist weiterhin nur ein CLI-Platzhalter.

## Festes Ausgabeformat für später

- Nur die acht definierten CPU-v2-Opcodes verwenden.
- Jede Anweisung ergibt ein 8-Bit-Wort aus 4-Bit-Opcode und 4-Bit-Operand.
- Das ROM besitzt genau 16 Adressen (`0x0` bis `0xF`).
- Mehr als 16 Anweisungen sind ein **Fehler**, niemals still kürzen.
- Für fehlende Adressen soll der generierte `match address`-Block
  mit `_ => 0xC0` (NOP) enden.
- Der Parser erkennt Labelzeilen; Pass 1 ordnet ihnen Adressen zu.
  Referenzen aus Sprungbefehlen werden erst in Pass 2 aufgelöst.

`examples/countdown.asm` liefert die Beispielbefehle und Labels für
Pass 1; die spätere Spade-ROM-Ausgabe kommt in einer weiteren Übung.
