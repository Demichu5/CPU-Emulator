# Emulator CPU – README

Poniżej znajduje się kompletna dokumentacja: opis projektu, struktura plików, zasada działania CPU, assembler, instrukcje, pipeline oraz sposób użycia.

---

# 1. Opis projektu
Projekt to prosty emulator CPU napisany w Pythonie. Zawiera:
- **emulator CPU** z rejestrami, flagami, ALU i stosem,
- **własny asembler** konwertujący instrukcje tekstowe na kod maszynowy,
- **loader** do wczytywania binariów, #NIE ZAIMPLEMENTOWANY
- **mini‑ISA** (instruction set architecture), którą można rozszerzać.

CPU działa w logice par instrukcja+argument i inkrementuje licznik programu o 2 bajty.

---

# 2. Struktura plików
```
cpu.py        – emulator procesora
compilator.py – asembler (tekst → opcode → bin)
loader.py     – loader do wczytywania binarnych danych
asm.txt       – przykładowy program w assemblerze
bin2.txt      – wygenerowany kod maszynowy
```

---

# 3. Model CPU
## Rejestry
Uniwersalne:
```
r1–r8
```
Specjalne:
```
s1–s6
pc – licznik programu
```

## ALU
Dwa rejestry operacyjne:
```
a
b
```

## Flagi
```
zero
carry
neg
```

## Pamięć
- RAM: 1024 bajty
- stack: Pythonowa lista imitująca stos

---

# 4. Cykl pracy CPU
1. Odczytaj bajt instrukcji (pc)
2. Odczytaj argument instrukcji (pc+1)
3. Wykonaj instrukcję
4. Zwiększ PC o 2 (chyba że instrukcja zmienia PC)

---

# 5. Asembler – opis działania
Asembler (`compilator.py`) robi trzy rzeczy:
1. **loaderBinFile()** – ładuje kod źródłowy (tokeny instrukcji) do listy.
2. **transcode()** – zamienia tokeny na wartości liczbowe według mapy `OPCODELIST`.
3. **saveToFile()** – zapisuje wynik do `bin2.txt` jako kod maszynowy linia po linii.

Przykład:
```
set r1
mov 13
```
→
```
1
1
2
13
```

---

# 6. Instrukcje – dokumentacja assemblera
Poniżej znajduje się pełna dokumentacja na podstawie plików źródłowych.

## 6.1 Instrukcje danych
### `set <rejestr>` – 1
Ustawia aktywny rejestr, który przyjmie dane w kolejnych instrukcjach.
```
set r1
```

### `mov <wartość>` – 2
Wpisuje liczbę do aktywnego rejestru.
```
set r1
mov 25
```

### `store <adres>` – 4
Zapisuje wartość z aktywnego rejestru do RAM.
```
set r1
mov 99
store 140
```

### `load <adres>` – 5
Ładuje wartość z RAM do aktywnego rejestru.
```
set r3
load 140
```

---

## 6.2 Operacje arytmetyczne (ALU)
### `alu_add_a <rejestr>` – 17
Ustawia operand **a** i zapamiętuje rejestr docelowy.

### `alu_add_b <rejestr>` – 18
Ustawia operand **b** i wykonuje `a = a + b`, zapisując wynik do rejestru ustawionego w `alu_add_a`.

Przykład:
```
set r1
mov 13
set r2
mov 14
alu_add_a r1
alu_add_b r2
# wynik: r1 = 27
```

### `alu_sub_a <rejestr>` – 19
Jak wyżej, ale przygotowuje operand `a` dla odejmowania.

### `alu_sub_b <rejestr>` – 20
Wykonuje: `a = a - b`.
```
set r1
mov 27
set r2
mov 14
alu_sub_a r1
alu_sub_b r2
# wynik: r1 = 13
```

---

## 6.3 Kontrola przepływu programu
### `jmp <adres>` – 33
Skok bezwarunkowy.
```
jmp 40
```

### `jmp_z <adres>` – 34
Skok jeśli flaga ZERO = 0.

### `jmp_c <adres>` – 35
Skok jeśli flaga CARRY = 0.

### `jmp_n <adres>` – 36
Skok jeśli flaga NEG = 0.

### `cmp <ra> <rb>` – 37 / 38
Porównuje dwa rejestry i ustawia flagi:
- ZERO
- CARRY
- NEG

### `inc` – 39
Inkrementacja aktywnego rejestru.
```
set r1
inc
```

### `call <adres>` – 51
Zapisuje aktualny `pc` na stos i skacze do adresu.

### `ret` – 52
Zdejmuje `pc` ze stosu.

### `halt` – 0
Zatrzymuje CPU.

### `nop` – 255
Nie robi nic.

---

# 7. Przykład programu
```
set r1
mov 1
set r2
mov 3
alu_add_a r1
a lu_add_b r2
halt
```

---

# 8. Uruchamianie CPU
```
python cpu.py
```
CPU będzie wykonywać program zapisany w pamięci ROM z pliku.

---

# Koniec dokumentacji

