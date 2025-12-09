📜 CPU 2021: (Projekt z Technikum)
To repozytorium jest cyfrowym archiwum mojego pierwszego, autorskiego projektu architektury procesora. Projekt powstał w czasach Technikum, kiedy moje zainteresowania zaczęły skręcać w stronę low-level programming oraz hardware'u.
Geneza Projektu

To nie jest kolejny klon procesora 6502, Z80.

    Architektura nie była inspirowana żadnym istniejącym procesorem (x86/ARM).

    Całość jest wynikiem moich własnych przemyśleń i wniosków wyciągniętych z analizy filmów o działaniu tranzystorów i logiki cyfrowej.

    Kod i schematy powstawały pierwotnie w zeszycie szkolnym, a następnie zostały ręcznie przepisane do Notatnika jako prototyp.

Architektura (Specyfikacja 2021)

Projekt zakładał unikalną, 8-bitową maszynę o specyficznym podejściu do rejestrów i przesyłu danych.
Główne Założenia

    Szyna Danych: 8-bit

    Pamięć RAM: Model "kartkowy" / liniowy

    Jednostka czasu: W tamtym czasie dla uproszczenia zakładałem 1 Cykl Zegara (Cz) ~ 1 sekunda (symulacja ręczna).

Rejestry "Typowane"

Ciekawostką tej architektury było przypisanie "typów danych" do konkretnych rejestrów sprzętowych (podejście wysokopoziomowe w hardware):

    R1: Integer (int)

    R2: Float / Liczby ujemne

    R3: Word (Słowa)

    R5: Syscall (Komunikacja z systemem)

Lista Instrukcji (ISA)

Instrukcje opierały się na czytelnych, "ludzkich" mnemonikach, np.:

    PUT (Wyślij):

        PUTr: Dane -> Rejestr (np. PUTr-205-R1)

        PUTm: Rejestr -> RAM

        PUTh: Rejestr -> Dysk

    GET (Odbierz):

        GETm: RAM -> Rejestr

    BUS (Magistrala): Bezpośrednie sterowanie przepływem danych.

    GATE / ALU: Operacje logiczne i arytmetyczne (GATEad, ALUadd).

Ewolucja

Ten projekt był fundamentem. Dziś, bogatszy o wiedzę i doświadczenie, rozwijam w pełni funkcjonalny emulator w języku C na mikrokontrolery Raspberry Pi Pico, ale logika CPU 2021 pozostaje dowodem na to, jak ewoluowało moje myślenie inżynierskie.****
