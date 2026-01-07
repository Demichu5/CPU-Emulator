; 02_variables.asm
; Cel: Pobrac 'licznik', dodac 1 i zapisac w 'wynik'.

.data
    var licznik 10
    var wynik

.code
fn MAIN
    ptr licznik
    load r1 PTR

    inc r1
    ptr wynik
    
    store r1 PTR

    halt