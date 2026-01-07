; loop.asm
; Odliczanie od 5 do 0.

.code
fn MAIN
    set r1
    mov 5
    
    set r0
    mov 0

PETLA:
    dec r1
    cmp r1 r0
    
    ; Jeśli R1 != 0 (czyli flaga ZERO == 0), skaczemy do PETLA
    ; jmp_z w Twoim CPU oznacza "Jump if Zero Flag is 0" (czyli "Not Zero")
    jmp_z PETLA

    halt