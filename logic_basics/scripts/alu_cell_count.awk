# Letzte Zellzahl des gewählten Top-Moduls aus dem Yosys-Protokoll lesen.
# Die Zellzahl kann mehrfach erscheinen, z. B. während der Synthese.
$0 == "=== " top " ===" {
    in_top = 1
    next
}
in_top && /Number of cells:/ {
    cells = $NF
    in_top = 0
}
in_top && /^[[:space:]]*[0-9]+ cells/ {
    cells = $1
    in_top = 0
}
END {
    if (cells == "") {
        print "Zellzahl nicht gefunden, bitte Yosys-Protokoll prüfen"
        exit 1
    }
    print cells
}
