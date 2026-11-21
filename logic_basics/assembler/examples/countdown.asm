; CPU-v2-Programm aus rom16x8.spade, zunächst als spätere Parser-Eingabe.
; Jede Anweisung belegt genau eine ROM-Adresse. Maximal 16 Wörter.
start:
    LDI 5
loop:
    ADD 15
    JZ alarm
    JMP loop
    NOP
alarm:
    OR 15
    AND 10
    XOR 15
    NOP
    JMP start
