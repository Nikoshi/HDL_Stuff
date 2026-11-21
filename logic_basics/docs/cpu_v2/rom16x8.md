# CPU v2, Kapitel VIII: 16×8-Bit-Programmspeicher

**Status:** Offene TDD-Übung. Die CPU-v2-Zustandssteuerung ist abgeschlossen.

## 1. Aufgabe des ROM

Der Programmzähler (`PC`) enthält die Adresse des nächsten Befehls.
Unser Programmspeicher liefert zu dieser Adresse das **vollständige
8-Bit-Instruktionswort**, das anschließend durch `decoder8` in
Opcode und Immediate zerlegt wird.

```text
      gespeicherter PC
            │ 4 Bit (0..15)
            ▼
      ┌───────────┐
      │ ROM 16×8  │
      └───────────┘
            │ 8 Bit
            ▼
        decoder8
        ├─ Opcode (4 Bit)
        └─ Immediate (4 Bit)
```

Der ROM ist in dieser Übung **rein kombinatorisch**: keine
Register, kein Takt, kein Reset und keine Schreibschnittstelle.
Wenn die Adresse wechselt, ändert sich nach kombinatorischer
Propagation der ausgegebene Befehl. Ein echtes FPGA könnte
Speicher anders implementieren; hier beschreiben wir bewusst
eine kleine, feste Lookup-Tabelle.

## 2. Die zwei Dimensionen: 16 Adressen und 8 Datenbits

Mit `address: uint<4>` lassen sich 16 Wörter auswählen.
Jedes Wort enthält genau acht Bits:

```text
    Bit 7 6 5 4 | 3 2 1 0
        O O O O | I I I I
         Opcode | Immediate
```

Beispiel: `0x95 = 1001_0101` bedeutet `JZ 5`.
Das obere Nibble `1001` ist der Opcode; das untere
Nibble `0101` ist die absolute Sprungadresse.

Die CPU v1 hatte ein **16×6-Bit-ROM**. Dessen Programm und
6-Bit-Instruktionsformat bleiben unverändert. CPU v2
bekommt eine eigene Datei und eine eigene Testbench.

## 3. Verbindliches Testprogramm

Wir speichern das im [ISA-Kapitel](isa.md) entworfene Programm,
das von fünf herunterzählt und danach drei LED-Muster durchläuft.

| Adresse (PC) | Wort | Befehl | Wirkung |
|---:|---|---|---|
| 0 | `0x45` | LDI 5 | Countdown starten |
| 1 | `0x0F` | ADD 15 | ACC minus 1, modulo 16 |
| 2 | `0x95` | JZ 5 | Bei ACC=0 zum Alarm springen |
| 3 | `0x81` | JMP 1 | Sonst weiterzählen |
| 4 | `0xC0` | NOP | Nicht erreichbarer Platzhalter |
| 5 | `0x2F` | OR 15 | LED-Muster 1111 |
| 6 | `0x1A` | AND 10 | LED-Muster 1010 |
| 7 | `0x3F` | XOR 15 | LED-Muster 0101 |
| 8 | `0xC0` | NOP | Muster einen Takt halten |
| 9 | `0x80` | JMP 0 | Countdown erneut starten |
| 10–15 | `0xC0` | NOP | Unbenutzte Programmadressen |

**NOP ist hier ein echtes, fest gespeichertes Maschinenwort.**
Es gibt keine uninitialisierten ROM-Adressen.

Das ist noch **kein neuer ISA-Entwurf**. Die Wortwerte
sind der Vertrag zwischen Softwaremodell, ROM-Test und
der späteren CPU-Integration.

## 4. Schnittstelle und Spade-Aufgabe

Datei: `logic_basics/src/cpu_v2/rom16x8.spade`

```spade
#[no_mangle(all)]
pub entity rom16x8_top(
    address: uint<4>,
    instruction: inv uint<8>
) {
    // Die 16 festen Befehlswörter werden hier decodiert.
}
```

Die Datei enthält bereits einen kompilierbaren Platzhalter,
der immer `0xC0` zurückgibt. Die Aufgabe ist, für jede
Adresse das in der Tabelle festgelegte Wort auszugeben.

Geeignete Spade-Sprachmittel kennst du aus
`rom16x6.spade`: beispielsweise ein `match` über
`address`. Die 8-Bit-Literale sollten eine eindeutige
Breite erhalten, etwa `0b1001_0101u8` für `0x95`.

**Bewusst nicht Teil dieser Aufgabe:** Opcode-Auswertung,
ACC/PC-Register, ALU-Berechnung und Programmdurchlauf.
Der ROM kennt nur Adresse und Befehlswort.

## 5. Tests

Im Repository-Root:

```sh
make test_rom16x8
```

Die unabhängige Testbench `logic_basics/test/rom16x8.cc`
prüft die 16 Referenzwörter sowie unmittelbare Adresswechsel
ohne Takt:

- 16 aufsteigende und 16 absteigende Adressen,
- alle 16×16 Adressübergänge mit jeweils zwei Beobachtungen,
- 64 zusätzliche Adresswechsel.

Das ergibt **608 Vergleiche**. Der aktuelle NOP-Platzhalter
muss bei nicht-NOP-Adressen fehlschlagen.
Der VCD-Trace liegt unter `logic_basics/build/rom16x8.vcd`.

`make test_completed` enthält weiterhin die **21 abgeschlossenen
Hardwareübungen**; `make test` führt zusätzlich den offenen
ROM-Test aus.

## 6. Danach

Anschließend sind die Bauteile für den CPU-v2-Top-Level
vorhanden: PC/ACC-Zustand, ROM, Felddecoder, Steuerfunktion
und die bereits getestete ALU. Der nächste Schritt besteht
darin, deren Verbindungen zu planen und eine gesamte
Ausführung gegen ein unabhängiges Softwaremodell zu testen.

Siehe auch [Kapitel VII: Registerzustände](state_v2.md).

---

[← Zurück: PC-/ACC-Zustand](state_v2.md) · [Zur Übersicht](../../../README.md)
