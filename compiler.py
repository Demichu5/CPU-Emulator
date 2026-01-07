import sys

# --- KONFIGURACJA OPCODÓW ---
OPCODELIST = {
    "set":  0x11, "mov":  0x12, "cpy":  0x13,
    "ptr":  0x14, "ptrH": 0x15, "ptrL": 0x16,
    "load": 0x17, "store":0x18, "push": 0x19, "pop":  0x1A,
    "add":  0x21, "sub":  0x22, "cmp":  0x23, "inc":  0x24, "dec":  0x25,
    "setloop": 0x31, "endloop": 0x32, 
    "jmp":     0x33, "jmp_z":   0x34, "jmp_c":   0x35, "jmp_n":   0x36,
    "call":    0x37, "ret":     0x38, "halt":    0x00, 
    "LOOP":    0x81, "PTR":     0x82,
}

REJESTRY = {
    "r0":0x00, "r1":0x01, "r2":0x02, "r3":0x03, "r4":0x04, "r5":0x05, "r6":0x06, "r7":0x07,
    "r8":0x08, "r9":0x09, "r10":0x0A, "r11":0x0B, "r12":0x0C, "r13":0x0D, "r14":0x0E, "r15":0x0F
}

# --- ZMIENNE GLOBALNE KOMPILATORA ---
tokens = []
VAR_STACK = {}      # Zmienne
FN_STACK = {}       # Funkcje (fn)
LABEL_STACK = {}    # NOWOŚĆ: Etykiety lokalne (Nazwa:)
VAR_DATA_LIST = [] 

def loadAsmFile(fileName, table):
    try:
        with open(fileName, "r") as filePTR:
            for line in filePTR:
                clean_line = line.split(';')[0].strip()
                if not clean_line: continue
                for opcode in clean_line.split():
                    table.append(opcode)
    except FileNotFoundError:
        print("Błąd: Nie znaleziono pliku.")
        sys.exit(1)

def parse_arg(arg_str):
    clean_arg = arg_str.replace("[", "").replace("]", "")
    
    # Kolejność sprawdzania:
    if clean_arg in REJESTRY: return REJESTRY[clean_arg]
    if clean_arg.upper() == "LOOP": return OPCODELIST["LOOP"]
    if clean_arg.upper() == "PTR": return OPCODELIST["PTR"]
    
    # Adresy:
    if clean_arg in FN_STACK: return FN_STACK[clean_arg]
    if clean_arg in VAR_STACK: return VAR_STACK[clean_arg]
    if clean_arg in LABEL_STACK: return LABEL_STACK[clean_arg] # NOWOŚĆ: Szukamy etykiet
    
    try: return int(clean_arg)
    except: return 0

# --- FAZA 1: Liczenie adresów i mapowanie etykiet ---
def firstPhase():
    _pc = 0
    i = 0
    _inCodeSection = False

    while(i < len(tokens)):
        token = tokens[i]

        # Sekcje
        if(token == ".data"): _inCodeSection = False; i += 1
        elif(token == ".code"): _inCodeSection = True; i += 1
        
        # NOWOŚĆ: Wykrywanie etykiet (np. "ETYKIETA:")
        elif token.endswith(":"):
            label_name = token[:-1] # Usuń dwukropek
            LABEL_STACK[label_name] = _pc
            # Etykieta nie zajmuje miejsca w pamięci, więc nie zwiększamy _pc
            i += 1
            continue

        # Zmienne
        elif(token == "var"):
            raw_name = tokens[i+1]; var_size = 1
            if "[" in raw_name and "]" in raw_name:
                name_part = raw_name.split("[")[0]
                size_part = raw_name.split("[")[1].replace("]", "")
                var_size = int(size_part); raw_name = name_part 
            
            init_bytes = [0] * var_size
            scan_offset = 2 
            if i + scan_offset < len(tokens):
                first_val_token = tokens[i + scan_offset]
                if first_val_token.startswith('"'):
                    clean_str = first_val_token.replace('"', '')
                    for idx, char in enumerate(clean_str):
                        if idx < var_size: init_bytes[idx] = ord(char)
                    scan_offset += 1
                elif "{" in first_val_token:
                    bytes_filled = 0
                    while True:
                        if i + scan_offset >= len(tokens): break
                        curr_token = tokens[i + scan_offset]
                        clean_token = curr_token.replace("{", "").replace("}", "")
                        if clean_token:
                            try:
                                val = int(clean_token)
                                if bytes_filled < var_size: init_bytes[bytes_filled] = val & 0xFF; bytes_filled += 1
                            except: pass
                        scan_offset += 1
                        if "}" in curr_token: break
            VAR_DATA_LIST.append((raw_name, var_size, init_bytes))
            i += scan_offset
        
        # Funkcje
        elif(token == "fn"):
            FN_STACK[tokens[i+1]] = _pc
            i += 2
        
        else:
            if(not _inCodeSection): i += 1; continue

            # Symulacja rozmiaru instrukcji
            if(token == "call"): _pc += 6; i += 2
            elif(token in ["jmp", "jmp_z", "jmp_c", "jmp_n"]):
                target_name = tokens[i+1]
                if target_name.upper() in ["LOOP", "PTR"]: _pc += 2
                else: _pc += 6
                i += 2
            elif(token in ["store", "load"]):
                target_name = tokens[i+2]
                if target_name.upper() == "PTR": _pc += 2
                else: _pc += 6
                i += 3
            elif(token in ["add", "sub", "cmp", "ptr", "cpy"]): _pc += 2; i += 3
            elif(token in ["set", "mov", "ptrH", "ptrL", "push", "pop", "inc", "dec"]): _pc += 2; i += 2
            elif(token in ["halt", "ret", "setloop", "endloop"]): _pc += 2; i += 1
            else: i += 1
            
    return _pc

# --- FAZA 2: Generowanie kodu ---
def secondPhase():
    binary = []
    i = 0
    _inCodeSection = False

    while(i < len(tokens)):
        token = tokens[i]
        
        if(token == ".code"): _inCodeSection = True; i += 1; continue
        if(token in [".data", "fn"]): i += 1; continue
        if token.endswith(":"): i += 1; continue # Ignoruj etykiety w 2. fazie

        if(token == "var"): # Pomiń var i jego argumenty
            # (Ta sama logika pomijania co w firstPhase, w skrócie)
            scan_offset = 2
            if i+2 < len(tokens):
                nxt = tokens[i+2]
                if nxt.startswith('"'): scan_offset = 3
                elif "{" in nxt: 
                    while "}" not in tokens[i+scan_offset]: scan_offset += 1
                    scan_offset += 1
            i += scan_offset
            continue

        if not _inCodeSection: i += 1; continue

        # --- INSTRUKCJE ---
        if token == "call":
            target_name = tokens[i+1]; address = parse_arg(target_name)
            binary.extend([OPCODELIST["ptrL"], address & 0xFF, OPCODELIST["ptrH"], (address >> 8) & 0xFF, OPCODELIST["call"], OPCODELIST["PTR"]])
            i += 2
        elif token in ["jmp", "jmp_z", "jmp_c", "jmp_n"]:
            target_name = tokens[i+1]
            if target_name.upper() in ["LOOP", "PTR"]:
                 binary.extend([OPCODELIST[token], parse_arg(target_name)])
            else:
                address = parse_arg(target_name)
                binary.extend([OPCODELIST["ptrL"], address & 0xFF, OPCODELIST["ptrH"], (address >> 8) & 0xFF, OPCODELIST[token], OPCODELIST["PTR"]])
            i += 2
        elif token in ["store", "load"]:
            reg_name = tokens[i+1]; target_name = tokens[i+2]
            if target_name.upper() == "PTR": binary.extend([OPCODELIST[token], parse_arg(reg_name)])
            else:
                address = parse_arg(target_name)
                binary.extend([OPCODELIST["ptrL"], address & 0xFF, OPCODELIST["ptrH"], (address >> 8) & 0xFF, OPCODELIST[token], parse_arg(reg_name)])
            i += 3
        elif token in ["add", "sub", "cmp", "ptr", "cpy"]:
            binary.extend([OPCODELIST[token], (parse_arg(tokens[i+1]) << 4) | parse_arg(tokens[i+2])]); i += 3
        elif token in ["set", "mov", "ptrH", "ptrL", "push", "pop", "inc", "dec"]:
            binary.extend([OPCODELIST[token], parse_arg(tokens[i+1])]); i += 2
        elif token in ["halt", "ret", "setloop", "endloop"]:
            binary.extend([OPCODELIST[token], 0x00]); i += 1
        else: i += 1

    return binary

def print_debug_view(bytecode):
    print(f"\n--- CODE DUMP (Code Size: {len(bytecode)} Bytes) ---")
    
    # Tworzymy odwrotną mapę rejestrów: {0: 'r0', 1: 'r1'...}
    # Dzięki temu zamienimy liczbę 0x00 na napis "r0"
    REV_REJESTRY = {v: k for k, v in REJESTRY.items()}

    pc = 0
    for i in range(0, len(bytecode), 2):
        if i+1 >= len(bytecode): break
        opcode = bytecode[i]
        arg = bytecode[i+1]
        
        # 1. Znajdź nazwę instrukcji (mnemonik)
        mnemonic = "???"
        mnemonic_key = "" # Do if-ów poniżej
        for k, v in OPCODELIST.items():
            if v == opcode: 
                mnemonic = k
                mnemonic_key = k
                break
        
        arg_str = f"0x{arg:02X}" # Domyślnie hex (dla mov, jmp itp.)

        # 2. Logika wyświetlania argumentów
        
        # A. Argumenty Specjalne
        if arg == 0x81: arg_str = "LOOP"
        elif arg == 0x82: arg_str = "PTR"
        
        # B. Instrukcje 2-rejestrowe (spakowane: 4 bity + 4 bity)
        # np. add r1 r2 -> 0x12 (High=1, Low=2)
        elif mnemonic_key in ["add", "sub", "cmp", "ptr", "cpy"]:
            reg1_val = (arg >> 4) & 0x0F
            reg2_val = arg & 0x0F
            r1_name = REV_REJESTRY.get(reg1_val, f"r{reg1_val}")
            r2_name = REV_REJESTRY.get(reg2_val, f"r{reg2_val}")
            arg_str = f"{r1_name} {r2_name}"

        # C. Instrukcje 1-rejestrowe (Argument to ID rejestru)
        # set r1 -> arg=1
        elif mnemonic_key in ["set", "push", "pop", "inc", "dec", "load", "store"]:
            # Pobierz nazwę z mapy, jeśli nie ma to "r?"
            if arg in REV_REJESTRY:
                arg_str = REV_REJESTRY[arg]
            else:
                # Zabezpieczenie na wypadek store r1 PTR (gdzie arg to rejestr)
                arg_str = f"r{arg}"

        # D. Instrukcje liczbowe (mov, ptrH, ptrL) i skoki zostają jako HEX
        # mov 10 -> mov 0x0A
        
        print(f"{pc:04X}: {opcode:02X} {arg:02X}  {mnemonic} {arg_str}")
        pc += 2

# --- GŁÓWNY PROGRAM ---
if __name__ == "__main__":
    # 1. Sprawdzenie argumentów
    if len(sys.argv) < 2:
        print("Błąd: Nie podano pliku wejściowego.")
        print("Użycie: python compiler.py <plik.asm>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = "program.bin" # Domyślna nazwa wyjściowa

    # 2. Wczytanie pliku
    tokens = []
    print(f"--- Kompilacja: {input_file} ---")
    loadAsmFile(input_file, tokens)

    # 3. Faza 1: Obliczenie rozmiaru kodu i mapowanie etykiet
    code_size = firstPhase()
    print(f"Rozmiar sekcji CODE: {code_size} (0x{code_size:X}) bajtów.")

    # 4. Nadanie adresów zmiennym (za kodem)
    current_var_addr = code_size
    for var_name, size, _ in VAR_DATA_LIST: 
        VAR_STACK[var_name] = current_var_addr
        print(f"Zmienna '{var_name}' -> Adres: 0x{current_var_addr:04X} (Rozmiar: {size})")
        current_var_addr += size

    # 5. Faza 2: Generowanie kodu binarnego
    bytecode = secondPhase()
    
    # Wyświetlanie debugowania (opcjonalne, można zakomentować dla czytelności)
    print_debug_view(bytecode)

    # 6. Padding i inicjalizacja danych
    print("\n--- SEKCJA DANYCH ---")
    for name, size, data in VAR_DATA_LIST:
        print(f"[{name}] Zapisuję: {data}")
        bytecode.extend(data)

    print(f"\nCałkowity rozmiar pliku binarnego: {len(bytecode)} bajtów.")

    # 7. Zapis do pliku
    with open(output_file, "wb") as f:
        f.write(bytearray(bytecode))
    
    print(f"Sukces! Wygenerowano plik: {output_file}")