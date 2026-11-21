# Mini-Assembler für CPU v2

Eigenständige C++17-Übung unterhalb von `logic_basics/`.
Die Spade-Hardware und die bestehende Hardware-Regression bleiben unverändert.

## Einstieg

1. [C++17-Kurzeinführung](../docs/assembler/cpp17_einstieg.md) lesen.
2. [Kapitel 1: Maschinenwort-Encoder](../docs/assembler/01_encoder.md) durcharbeiten.
3. `encoder.cpp` selbst implementieren, anschließend `make test_assembler_encoder` ausführen.

Im Repository-Root (Nix-Dev-Shell):

```sh
make test_assembler_encoder    # Offene Encoder-Übung (zunächst rot)
make assembler                 # Noch unvollständige CLI bauen
make test_completed            # Bestehende Hardware-Regression
```

`test/encoder_test.cpp` erwartet 131 Vergleiche. Das Executable
`logic_basics/build/mini_assembler` ist derzeit nur ein CLI-Platzhalter.

## Festes Ausgabeformat für später

- Nur die acht definierten CPU-v2-Opcodes verwenden.
- Jede Anweisung ergibt ein 8-Bit-Wort aus 4-Bit-Opcode und 4-Bit-Operand.
- Das ROM besitzt genau 16 Adressen (`0x0` bis `0xF`).
- Mehr als 16 Anweisungen sind ein **Fehler**, niemals still kürzen.
- Für fehlende Adressen soll der generierte `match address`-Block
  mit `_ => 0xC0` (NOP) enden.
- Sprunglabels erhalten später absolute Adressen; ihre Umsetzung
  ist noch **nicht** Teil der Encoder-Übung.

`examples/countdown.asm` ist das bekannte Testprogramm für den
späteren Parser und ROM-Generator. Es wird noch nicht verarbeitet.
