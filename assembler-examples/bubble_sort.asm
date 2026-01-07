; bubblesort.asm
; Dane wejściowe: {5, 1, 4, 2, 8}
; Oczekiwany wynik: {1, 2, 4, 5, 8}

.data
    var liczby[5] {5 1 4 2 8}

.code
fn MAIN
    set r5
    mov 20
    set r0
    mov 0

    set r15
    mov 0

PETLA_GLOWNA:
    cmp r5 r0
    jmp_z ROZPOCZNIJ_PRZEBIEG
    halt

ROZPOCZNIJ_PRZEBIEG:
    set r10
    mov liczby
    
    set r11
    mov liczby
    inc r11

    set r4
    mov 4

PETLA_WEWNETRZNA:
    cmp r4 r0
    jmp_z SPRAWDZ_PARE
    jmp NASTEPNY_PRZEBIEG_GLOWNY

SPRAWDZ_PARE:
    ptr r15 r10
    load r1 PTR
    ptr r15 r11
    load r2 PTR
    cmp r1 r2
    jmp_n ZROB_SWAP
    jmp DALEJ

ZROB_SWAP:
    ptr r15 r11
    store r1 PTR
    ptr r15 r10
    store r2 PTR

DALEJ:
    inc r10
    inc r11
    dec r4
    jmp PETLA_WEWNETRZNA

NASTEPNY_PRZEBIEG_GLOWNY:
    dec r5
    jmp PETLA_GLOWNA