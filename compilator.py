OPCODELIST = {
    "set": 1,
    "mov": 2,
    "store": 3,
    "load": 4,

    "alu_add_a": 17,
    "alu_add_b": 18,
    "alu_sub_a": 19,
    "alu_sub_b": 20,

    "jmp": 33,
    "jmp_z": 34,
    "jmp_c": 35,
    "jmp_n": 36,
    "cmp": 37,
    "inc": 38,

    "call": 51,
    "ret": 52,

    "halt": 0,
    "nop": 255,

    "r1": 1,
    "r2": 2,
    "r3": 3,
    "r4": 4,
    "r5": 5,
    "r6": 6,
    "r7": 7,
    "r8": 8,

    "s1": 9,
    "s2": 10,
    "s3": 11,
    "s4": 12,
    "s5": 13,
    "s6": 14
}

REJESTRLIST = {
    "r1": 1,
    "r2": 2,
    "r3": 3,
    "r4": 4,
    "r5": 5,
    "r6": 6,
    "r7": 7,
    "r8": 8,

    "s1": 9,
    "s2": 10,
    "s3": 11,
    "s4": 12,
    "s5": 13,
    "s6": 14
}


cache_raw = []
cache = []

def loaderBinFile(fileName, array, index = 0):
    with open(fileName, "r") as file:
        for line in file:
            for opcode in line.split():
                #print(f"index: {i+index}")
                #print(opcode)
                array.insert(index, opcode)
                index=index+1
    
    return array

def transcode(source_raw, destination):
    cache_lenght = len(source_raw)
    for opcode in source_raw:
        if(opcode in OPCODELIST):
            destination.append(OPCODELIST[opcode])

        else:
            destination.append(int(opcode))

def saveToFile(source):
    with open("bin2.txt", "a") as file:
        for opcode in source:
            file.writelines(str(opcode)+'\n')


loaderBinFile("asm.txt", cache_raw)
print(cache_raw)
transcode(cache_raw, cache)
print(cache)
saveToFile(cache)
