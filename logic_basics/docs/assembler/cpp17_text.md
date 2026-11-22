# C++17: Text zeilenweise und wortweise verarbeiten

Begleitmaterial für
[Kapitel II: Parser](02_parser.md).
Hier geht es um **Text**, nicht um Bitmuster.
Die Grundlagen zu Headerdateien und Ganzzahltypen
stehen bereits in der [ersten Einführung](cpp17_einstieg.md).

## 1. Eine Zeile als `std::string`

Ein String enthält eine Folge von Zeichen:

```cpp
#include <string>

std::string text = "eins zwei";
auto laenge = text.size();
auto ersteLeerstelle = text.find(' ');
std::string erstesWort = text.substr(0, ersteLeerstelle);
```

`find` liefert die erste Position oder
`std::string::npos`, wenn das Zeichen fehlt.
`substr(start, anzahl)` erzeugt einen Teilstring.
Das ist nützlich, um einen Kommentar zu entfernen,
bevor wir Wörter erkennen.

`std::string` speichert **keine Zahl**: Die Zeichen
`'1'` und `'5'` sind nicht automatisch der Wert 15.

## 2. Mit Streams Wörter abtrennen

Ein `std::istringstream` liest aus einer Zeichenfolge
ähnlich wie `std::cin` aus der Konsole.
`operator>>` überspringt Leerzeichen, Tabs
und andere Leerraumzeichen.

```cpp
#include <sstream>
#include <string>
#include <iostream>

std::istringstream quelle("name  wert");
std::string a, b, c;

quelle >> a >> b;
if (quelle >> c) {
    std::cerr << "Ein drittes Wort war nicht erwartet\n";
}
```

Bei `a` steht anschließend `"name"`, bei `b`
`"wert"`. Das Erkennen zusätzlicher Wörter
ist wichtig: `"ADD 1 2"` darf nicht unbemerkt
so behandelt werden, als stünde nur `"ADD 1"` dort.

## 3. Zeichen einzeln prüfen

Bei Labelnamen hilft eine Schleife über
`std::string`:

```cpp
std::string wort = "abc";
for (char zeichen : wort) {
    // Ein Zeichen nach dem anderen untersuchen.
}
```

`<cctype>` stellt dafür `std::isalpha`,
`std::isdigit` und `std::isalnum` bereit.
Alternativ kann man für unsere festgelegte
ASCII-Grammatik auch konkrete Zeichenbereiche
wie `'a'` bis `'z'` prüfen.

Beachte: Bei den Funktionen aus `<cctype>`
solltest du das Zeichen zu `unsigned char`
umwandeln, bevor es an die Funktion geht,
damit negative `char`-Werte nicht zu
undefiniertem Verhalten führen.

## 4. Ein Ergebnis mit mehreren Feldern

`struct` bündelt zusammengehörige Daten:

```cpp
struct Wetter {
    std::string ort;
    int temperatur = 0;
};
Wetter messung;
messung.ort = "Oslo";
messung.temperatur = 2;
```

Unser `ParsedLine`-Typ ist genau so ein
Datenpaket. Das Feld `kind` legt fest,
welche übrigen Angaben relevant sind.
Die Voreinstellungen in `parser.hpp`
ermöglichen eine leere Struktur durch `return {};`.

## 5. Erst Zeilen, dann Programmdateien

`std::getline` liest eine ganze Zeile,
nicht nur das nächste Wort. Eine spätere
Dateiverarbeitung könnte so beginnen:

```cpp
#include <fstream>
#include <string>

std::ifstream datei("werte.txt");
std::string zeile;
while (std::getline(datei, zeile)) {
    // Hier könnte eine Funktion die ganze Zeile auswerten.
}
```

Dieses Beispiel ist **nicht** die aktuelle Aufgabe.
`parse_line` bekommt bereits eine einzelne Zeile.
So lässt sich die Parserlogik testen, ohne
zuerst Datei-I/O oder eine Kommandozeilenoberfläche
bauen zu müssen.

## 6. Fehler korrekt unterscheiden

`std::invalid_argument` bedeutet:
*Die Eingabeform ist nicht erlaubt.*
`std::out_of_range` bedeutet:
*Ein numerischer Wert passt nicht in den Bereich.*

Eine verständliche Fehlermeldung bei einer
Exception ist hilfreich, z. B. mit dem
unbekannten Befehlsnamen. Der Parser-Test
prüft zunächst den **Exceptiontyp**, nicht
die genaue Formulierung des Texts.

---

[Zur Parser-Aufgabe](02_parser.md) ·
[Weiter: Streams und Collections](cpp17_collections.md)
