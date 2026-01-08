# Emulator CPU – README

Demichu5 CPU Architecure - Reference Manual
Wersja 1.0 (Rev. 2026) Architektura 8-Bit

```
Przetestowany GCC/TCC
tcc cpu.c filehandler.c -o main
python3 compiler.py <program.asm>
./main <program.bin>
```
---

Spis treści
```
1: Model Programowy:
	1.1 Organizacja pamięci
	1.2 Rejestry ogolnego przeznaczenia
	1.3 Rejestry Specjalne i wskaznikowe
	1.4 Rejestr stanu

2: Format Instrukcji
	2.1 Kodowanie instrukcji
	2.2 Tryby adresowania

3: Zestaw Instrukcji (ISA)
	3.1 Przesyl danych
	3.2 Arytmeryka i logika
	3.3 Sterowanie przepływem

4: Dyrektywny Assemblera

5: Przykladowy program (Bubble Sort)

6. Licencjonowanie Komercyjne
```
---

# 1. Model Programowy
```
1.1 Organizacja pamięci
	Procesor operuje na przestrzeni adresowej o rozmiarze 64 KB (65 536 Bajtow)
	Architektura typu Von Neumann - Kod programu i dane wyspoldziela te sama przesten adresowa
	*Slowo danych: 8 Bitow (1 Bajt)
	*Szyna adresowa: 16 Bitow (2 Bajty)
	*Kolejnosc bajtow: Little-Endian

1.2 Rejestry ogolnego przeznaczenia
	Procesor udostepnia 16 rejestrow 8-bitowych dostępnych dla programisty
	R0 - R15 : Sluza do przechowywania danych tymczasowych, argumentow arytmerycznych i licznikow petli

1.3 Rejestry specjalne i wskaźnikowe
	PC : 16-bit (2 Bajty) : Program Counter wskazuje adres następnej instrukcji do wykonania, inkrementowany automatyczne (+2) po każdym cyklu fetch
	PTR : 16-bit (2 Bajty) : Pointer Register sluzy do adresowania używanych operacji load/store. Sklada się z dwóch części 8-bitowych (High/Low)

1.4 Rejestr Stanu
	Flagi sa aktualizowane automatycznie po operacjach arytmerycznych (ADD, SUB, INC, DEC) oraz porównania (CMP)
	Flaga Z : ZERO : Ustawiona (1), gdy wynik operacji wynosi 0
	Flaga C : CARRY : Ustawiona (1), gdy nastapilo przepełnienie (wynik > 255)
	Flaga N : NEGATIVE : Ustawiona (1), gdy wynik jest ujemny
```

# 2. Format Instrukcji
```
Wszystkie instrukcje maja stala dlugosc 2 Bajtow (16 bitow)
2.1 Kodowanie instukcji
	[ 8 bitow : OPCODE ] [ 8 bitow : ARGUMENT ]
	Sposób interpretacji pola ARGUMENT zależy od instrukcji:
		1: Immediate: Cale 8 Bitow to liczba (np. mov 10)
		2: Register: Mlodsze 4 bit to indeks rejestru (np. set r1)
		3: Register-Register: Dwa rejestry spakowane w jeden bajt (High Nibble = Dest, Low Nibble = Src) (np. add r1 r2)
```

# 3. Zestaw Instrukcji (ISA)
```
Legenda:
	act : Aktywny rejestr (wybierany instrukcja set)
	rD : Rejestr docelowy
	rS : Rejestr zrodlowy
	imm8 : Wartosc natychmiastowa 8-Bit
	[PTR] : Wartosc w pamięci pod adresem wskaznywanym przez rejestr PTR
```
```
3.1 Przesyl Danych
	Mnemonik	Opcode		Operand		Opis Dzialania
	set			0x11		rD			wybiera rD jako aktywny rejestr dla operacji mov
	mov			0x12		imm8		act = imm8
	cpy			0x13		rD rS		rD = rS
	ptr			0x14		rH rL		PTR_High = rH, PTR_Low = rL
	ptrh		0x15		imm8		PTR_High = imm8
	ptrl		0x16		imm8		PTR_Low	= imm8
	load		0x17		rD			rD = MEM[PTR]
	store		0x18		rS			MEM[PTR] = rS
	push		0x19		rS			STACK[SP++] = rS
	pop			0x1A		rD			rD = STACK[--SP]

3.2 Arytmeryka i Logika (ALU)
	Mnemonik	Opcode		Operand		Flagi		Opis Dzialania
	add			0x21		rD rS		Z, C		rD = rD + rS
	sub			0x22		rD rS		Z, N		rD = rD - rS
	cmp			0x23		rD rS		Z, N, C		wykonuje rD - rS, aktualizuje flagi, nie zmienia rD
	inc			0x24		rD			Z			rD = rD + 1
	dec			0x25		rD			Z			rD = rD - 1

3.3 Sterowanie Przeplywem
	Mnemonik	Opcode		Operand		Warunek Skoku		Opis Dzialania
	jmp			0x33		Label		-					Skok bezwarunkowy, PC = Label
	jmp_z		0x34		Label		Z == 0				Skocz jeśli NIE ZERO (wynik != 0)
	jmp_c		0x35		Label		C == 0				Skocz jeśli BRAK PRZENIESIENIA
	jmp_n		0x36		Label		N == 0				Skocz jeśli DODATNI (wynik >= 0)
	call		0x37		Label		-					STACK[SP++] = PC, PC = Label
	ret			0x38		-			-					PC = STACK[--SP]
	halt		0x00		-			-					Zatrzymuje zegar procesora
```

# 4. Dyrektywy Assemblera
```
	Kompilator obsluguje nastepujace sekcje i definicje danych
	Sekcje:
		.code - rozpoczyna sekcje intrukcji
		.data - rozpoczyna sekcje definicji zmiennych (nie wykonywalna)
	Deklaracja zmiennych (var)
		; Skladnia: var <nazwa> <rozmiar_opcjonalny> <wartosc_opcjonalna>
		var liczba			; Zmienna bez wartosci
		var licznik 10			; Zmienna o wartości 10
		
		var buffor[10]			; Tablica o rozmiarze 10 wypelniona zerami
		var tablica[5] {1 2 3 4 5}	; Tablica o rozmiarze 5 wypelniona danymi 1 2 3 4 5
		var napis "Tekst"		; Tablica o rozmiarze N + 1, ciąg znakow + null terminator

	Etykiety i Funkcje
	; Etykieta skoku
	NAZWA:
	
	; Funkcje dla call
	fn NAZWA_FUNKCJI
```

5. Przykalowy program (Bubble Sort)
```
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
```

6. Licencja
```
Copyright (C) 2025 Demichu5

Ten projekt jest oprogramowaniem wolnym: możesz go rozpowszechniać i/lub modyfikować zgodnie z warunkami Powszechnej Licencji Publicznej GNU (GPL) opublikowanej przez Free Software Foundation, w wersji 3 tej Licencji lub (według Twojego wyboru) którejkolwiek z późniejszych wersji.
Licencjonowanie Komercyjne

Jeśli chcesz wykorzystać ten projekt w produkcie zamkniętym (proprietary) lub nie możesz spełnić warunków licencji GPLv3, dostępna jest licencja komercyjna. Proszę o kontakt w celu ustalenia warunków.
```
