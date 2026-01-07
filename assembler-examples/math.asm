; math.asm
; Proste dodawanie: 15 + 20
; Wynik: R1 = 35

.code
fn MAIN
    set r1
    mov 15
    set r2
    mov 20
    add r1 r2
    halt