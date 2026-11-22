# C++17: Streams, Vektoren und Symboltabellen

Begleitmaterial zu
[Kapitel III: Erster Programmdurchlauf](03_pass1.md).
Hier werden neue Datenstrukturen an unabhängigen Beispielen erklärt,
ohne die Assembler-Implementierung vorwegzunehmen.

## 1. `std::istream&` und `std::getline`

`std::istream` ist der gemeinsame Eingabetyp für
Dateien (`std::ifstream`), Text im Speicher (`std::istringstream`)
und andere Eingabequellen.

```cpp
#include <istream>
#include <string>

void zeilen_zaehlen(std::istream& eingabe) {
    std::string zeile;
    unsigned anzahl = 0;
    while (std::getline(eingabe, zeile)) {
        ++anzahl;
    }
}
```

`std::getline` liest die gesamte nächste Zeile.
Dabei wird der abschließende Zeilenumbruch entfernt.
Das `&` bedeutet, dass die Funktion denselben Stream nutzt,
statt eine eigene Kopie anzulegen. Streams sind nicht
wie normale Strings kopierbar.

Für einfache Tests ohne Dateien:

```cpp
#include <sstream>

std::istringstream input("eins\nzwei\n");
zeilen_zaehlen(input);
```

## 2. `std::vector` für Reihenfolgen

`std::vector<T>` speichert eine dynamisch wachsende Folge
von Elementen des Typs `T`:

```cpp
#include <vector>
#include <string>

std::vector<std::string> einkauf;
einkauf.push_back("Brot");
einkauf.push_back("Milch");

auto anzahl = einkauf.size();    // 2
auto erstes = einkauf[0];        // "Brot"
```

Die Indizes beginnen bei 0. `push_back` hängt ein Element
an. `size()` zählt Elemente, nicht Bytes. Für unsere
Befehlsfolge ist die Position im Vektor die ROM-Adresse.

`std::size_t` aus `<cstddef>` ist ein
vorzeichenloser Ganzzahltyp für Größen und Indizes.
Er passt zum Rückgabetyp von `std::vector::size()`.

## 3. `std::unordered_map` für Namen

Eine Map verbindet einen Schlüssel (z. B. einen Namen)
mit einem Wert (z. B. einer Anzahl).

```cpp
#include <unordered_map>
#include <string>

std::unordered_map<std::string, int> alter;
alter.emplace("Lena", 21);
alter.emplace("Tom", 19);
```

Die Einfügereihenfolge ist nicht die spätere
Ausgabereihenfolge. Für die Befehle verwenden wir
deshalb den `vector`; die Map dient nur als Nachschlagetabelle.

### Doppelte Namen erkennen

`emplace` liefert ein Paar:
einen Iterator zum Eintrag und ein `bool`,
ob tatsächlich ein neuer Eintrag angelegt wurde.

```cpp
auto [it, inserted] = alter.emplace("Lena", 99);

if (!inserted) {
    // "Lena" existierte schon; der erste Wert bleibt erhalten.
}
```

Diese `[it, inserted]`-Syntax heißt
**Structured Binding** und ist ab C++17 verfügbar.
Das Ergebnis ist keine komplette Assemblerlösung,
aber dieselbe Technik eignet sich für Symboltabellen.

Ein häufiger Fehler wäre dagegen, unkritisch
`alter["Lena"] = 99;` zu verwenden.
Das würde einen bestehenden Eintrag überschreiben.

## 4. Der Unterschied zwischen Zählung und Adresse

Eine Position ist die Anzahl der bereits gespeicherten
Elemente. Für unsere ROM-Größe gelten 0 bis 15 als
gültige Indizes. Ist die Vektorgröße bereits 16,
würde ein weiteres Element an Index 16 liegen.

Ein Beispiel ohne Assembler:

```cpp
std::vector<int> werte = {4, 7, 9};
auto naechster_index = werte.size(); // 3
```

Beachte den Unterschied: Eine volle Liste hat
`size() == 16`, obwohl der höchste gültige Index 15 ist.

## 5. Die Fehlerklassen

Wir nutzen in dieser Etappe zwei vorhandene
Standard-Exceptions aus `<stdexcept>`:

- `std::invalid_argument` für einen fehlerhaften Namen
  oder eine ungültige Syntax.
- `std::out_of_range` für einen Platz, der außerhalb
  unserer 16 adressierbaren ROM-Wörter läge.

Die Tests prüfen die Exceptiontypen, aber nicht die
genaue Formulierung der Meldungen.

---

[Zur Übung: Erster Durchlauf](03_pass1.md) · [Zur Kapitelübersicht](../README.md)
