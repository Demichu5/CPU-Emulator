.data
    ; Tablica nieposortowana {5, 1, 4, 2, 8}
    var liczby[5] {5 1 4 2 8} 
    
    ; Oczekiwany wynik po sortowaniu: 1, 2, 4, 5, 8

.code
fn MAIN
    ; R5 = Licznik główny (ile razy "przemielić" tablicę). 
    ; Dla pewności zrobimy n^2 przejść (5*5=25), żeby "bąbelki" na pewno wypłynęły.
    set r5
    mov 20
    
    ; R0 = 0 (pomocniczy do porównań)
    set r0
    mov 0

PETLA_GLOWNA:
    ; Sprawdź czy koniec licznik głównego
    cmp r5 r0
    ; Jeśli r5 != 0 (jmp_z = jump if not zero), to lecimy dalej
    jmp_z ROZPOCZNIJ_PRZEBIEG
    
    ; Jeśli r5 == 0, koniec
    jmp KONIEC_PROGRAMU

ROZPOCZNIJ_PRZEBIEG:
    ; --- RESET WSKAŹNIKÓW ---
    ; R10 = Wskaźnik LEWY (i)
    set r10
    mov liczby
    
    ; R11 = Wskaźnik PRAWY (i+1)
    set r11
    mov liczby
    inc r11

    ; R4 = Licznik par w jednym przebiegu (4 pary dla 5 liczb)
    set r4
    mov 4
    
    ; R15 = Adres wysoki (0)
    set r15
    mov 0

PETLA_WEWNETRZNA:
    ; Sprawdź czy sprawdziliśmy wszystkie pary (czy R4 == 0)
    cmp r4 r0
    
    ; Jeśli R4 != 0, sprawdzamy parę
    jmp_z SPRAWDZ_PARE
    
    ; Jeśli R4 == 0, koniec tego przebiegu
    jmp NASTEPNY_PRZEBIEG_GLOWNY

SPRAWDZ_PARE:
    ; Pobierz LEWY (R1)
    ptr r15 r10
    load r1 PTR

    ; Pobierz PRAWY (R2)
    ptr r15 r11
    load r2 PTR

    ; --- PORÓWNANIE ---
    ; Sortujemy rosnąco. Chcemy SWAP jeśli LEWY > PRAWY.
    ; Obliczamy: LEWY(R1) - PRAWY(R2)
    cmp r1 r2
    
    ; Jeśli R1 > R2, wynik jest DODATNI.
    ; Twoje jmp_n działa jako "Jump if Not Negative" (czyli jeśli dodatni).
    ; Więc skaczemy do SWAPa.
    jmp_n ZROB_SWAP
    
    ; Jeśli wynik ujemny (R1 < R2), jest ok, idziemy dalej bez zmian
    jmp DALEJ

ZROB_SWAP:
    ; Zapisz R1 (duża liczba) pod adres R11 (prawo)
    ptr r15 r11
    store r1 PTR

    ; Zapisz R2 (mała liczba) pod adres R10 (lewo)
    ptr r15 r10
    store r2 PTR

DALEJ:
    ; Przesuń wskaźniki w prawo
    inc r10
    inc r11
    
    ; Zmniejsz licznik par
    dec r4
    
    jmp PETLA_WEWNETRZNA

NASTEPNY_PRZEBIEG_GLOWNY:
    ; Zmniejsz licznik główny
    dec r5
    jmp PETLA_GLOWNA

KONIEC_PROGRAMU:
    halt