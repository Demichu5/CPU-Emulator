; functions.asm
; Wynik w R1 powinien wynosic: 0 -> 2 -> 4 -> 6.

.code
fn MAIN
    set r1
    mov 0
    call DODAJ_DWA
    call DODAJ_DWA
    call DODAJ_DWA
    halt

fn DODAJ_DWA
    inc r1
    inc r1
    ret