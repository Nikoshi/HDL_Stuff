# Logiktiefe aus der letzten Yosys-ltp-Zusammenfassung auslesen.
# Erwartet eine Zeile wie: Longest topological path in ... (length=12):
/Longest topological path in .*length=[0-9]+/ {
    if (match($0, /length=[0-9]+/)) {
        depth = substr($0, RSTART + 7, RLENGTH - 7)
    }
}
END {
    if (depth == "") {
        print "nicht gefunden (siehe Log)"
        exit 1
    }
    print depth
}
