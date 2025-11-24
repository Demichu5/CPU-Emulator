Core_cpu_ramSpace = [0]*1*1024
Core_cpu_stack = []

Core_cpu_ramSpace[0] = 1; Core_cpu_ramSpace[1] = 1      #set r1
Core_cpu_ramSpace[2] = 2; Core_cpu_ramSpace[3] = 0      #mov 0
Core_cpu_ramSpace[4] = 1; Core_cpu_ramSpace[5] = 2      #set r2
Core_cpu_ramSpace[6] = 2; Core_cpu_ramSpace[7] = 3      #mov 3
Core_cpu_ramSpace[8] = 1; Core_cpu_ramSpace[9] = 3      #set r3 
Core_cpu_ramSpace[10] = 2; Core_cpu_ramSpace[11] = 12   #mov 12

Core_cpu_ramSpace[12] = 19; Core_cpu_ramSpace[13] = 3   #alu_sub_a r3
Core_cpu_ramSpace[14] = 20; Core_cpu_ramSpace[15] = 2   #alu_sub_b r2

Core_cpu_ramSpace[16] = 1; Core_cpu_ramSpace[17] = 1    #set r1
Core_cpu_ramSpace[18] = 39; Core_cpu_ramSpace[19] = 0     #inc

Core_cpu_ramSpace[20] = 37; Core_cpu_ramSpace[21] = 1     #cmp_a r1
Core_cpu_ramSpace[22] = 38; Core_cpu_ramSpace[23] = 2     #cmp_b r2
Core_cpu_ramSpace[24] = 35; Core_cpu_ramSpace[25] = 12    #jmp_c 12

Core_cpu_ramSpace[26] = 1; Core_cpu_ramSpace[27] = 9        #set s1
Core_cpu_ramSpace[28] = 2; Core_cpu_ramSpace[29] = 100      #mov 100

Core_cpu_ramSpace[30] = 0; Core_cpu_ramSpace[31] = 0      #halt

Core_CPU_rejestrs = {
    #Hardware
    "pc": 0,

    #Uniwersalne
    "r1": 0,
    "r2": 0,
    "r3": 0,
    "r4": 0,
    "r5": 0,
    "r6": 0,
    "r7": 0,
    "r8": 0,

    #Specialne
    "s1": 0, # Interrupt Call
    "s2": 0, # 
    "s3": 0, # 
    "s4": 0, # 
    "s5": 0, # 
    "s6": 0  # 
}

Core_ALU_rejestrs = {
    "a": 0,
    "b": 0
}

Active_rejestrs_SET = {
    "set_rejestr": "",
    "set_alu_a": "",
    "set_alu_b": "",
    "set_cmp_a": "",
    "set_cmp_b": ""
}

Core_CPU_flags = {
    "zero": 0,
    "carry": 0,
    "neg": 0
}

def fetch(pc1, pc2):
    # 1 - set
    if(pc1 == 1):
        if(pc2 <= 8):
            Active_rejestrs_SET["set_rejestr"] = f"r{pc2}"
        else:
            Active_rejestrs_SET["set_rejestr"] = f"s{pc2-8}"

    # 2 - mov
    if(pc1 == 2):
        Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]] = pc2

    # 3 - cpy
    if(pc1 == 3):
        if(pc2 <= 8):
            Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]] = Core_CPU_rejestrs[f"r{pc2}"]
        else:
            Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]] = Core_CPU_rejestrs[f"s{pc2}"]

    # 4 - store
    if(pc1 == 4):
        Core_cpu_ramSpace[pc2] = Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]]

    # 5 - load
    if(pc1 == 5):
        Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]] = Core_cpu_ramSpace[pc2]

    # 17 - alu_add_a
    if(pc1 == 17):
        Active_rejestrs_SET["set_alu_a"] = f"r{pc2}"
        Core_ALU_rejestrs["a"] = Core_CPU_rejestrs[Active_rejestrs_SET["set_alu_a"]]
    
    # 18 - alu_add_b
    if(pc1 == 18):
        Active_rejestrs_SET["set_alu_b"] = f"r{pc2}"
        Core_ALU_rejestrs["b"] = Core_CPU_rejestrs[Active_rejestrs_SET["set_alu_b"]]

        Core_ALU_rejestrs["a"] = Core_ALU_rejestrs["a"] + Core_ALU_rejestrs["b"]
        Core_CPU_rejestrs[Active_rejestrs_SET["set_alu_a"]] = Core_ALU_rejestrs["a"]

    # 19 - alu_sub_a
    if(pc1 == 19):
        Active_rejestrs_SET["set_alu_a"] = f"r{pc2}"
        Core_ALU_rejestrs["a"] = Core_CPU_rejestrs[Active_rejestrs_SET["set_alu_a"]]

    # 20 - alu_sub_b
    if(pc1 == 20):
        Active_rejestrs_SET["set_alu_b"] = f"r{pc2}"
        Core_ALU_rejestrs["b"] = Core_CPU_rejestrs[Active_rejestrs_SET["set_alu_b"]]

        Core_ALU_rejestrs["a"] = Core_ALU_rejestrs["a"] - Core_ALU_rejestrs["b"]
        Core_CPU_rejestrs[Active_rejestrs_SET["set_alu_a"]] = Core_ALU_rejestrs["a"]

    # 33 - jmp
    if(pc1 == 33):
        Core_CPU_rejestrs["pc"] = pc2

    # 34 - jmp_z
    if(pc1 == 34):
        if(Core_CPU_flags["zero"] == 0):
            Core_CPU_rejestrs["pc"] = pc2

        Core_CPU_flags["zero"] = 0

    # 35 - jmp_c
    if(pc1 == 35):
        if(Core_CPU_flags["carry"] == 0):
            Core_CPU_rejestrs["pc"] = pc2

        Core_CPU_flags["carry"] = 0

    # 36 - jmp_n
    if(pc1 == 36):
        if(Core_CPU_flags["neg"] == 0):
            Core_CPU_rejestrs["pc"] = pc2

        Core_CPU_flags["neg"] = 0

    # 37 - cmp_a
    if(pc1 == 37):
        Active_rejestrs_SET["set_cmp_a"] = f"r{pc2}"

    # 38 - cmp_b
    if(pc1 == 38):
        Active_rejestrs_SET["set_cmp_b"] = f"r{pc2}"

        if(Core_CPU_rejestrs[Active_rejestrs_SET["set_cmp_a"]] - Core_CPU_rejestrs[Active_rejestrs_SET["set_cmp_b"]] == 0):
            Core_CPU_flags["zero"] = 1
        elif(Core_CPU_rejestrs[Active_rejestrs_SET["set_cmp_a"]] - Core_CPU_rejestrs[Active_rejestrs_SET["set_cmp_b"]] > 0):
            Core_CPU_flags["carry"] = 1
        elif(Core_CPU_rejestrs[Active_rejestrs_SET["set_cmp_a"]] - Core_CPU_rejestrs[Active_rejestrs_SET["set_cmp_b"]] < 0):
            Core_CPU_flags["neg"] = 1

    # 39 - inc
    if(pc1 == 39):
        Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]] = Core_CPU_rejestrs[Active_rejestrs_SET["set_rejestr"]] + 1

    # 51 - call
    if(pc1 == 51):
        Core_cpu_stack.append(Core_CPU_rejestrs["pc"]+2)
        Core_CPU_rejestrs["pc"] = pc2 - 2

    # 52 - ret
    if(pc1 == 52):
        Core_CPU_rejestrs["pc"] = Core_cpu_stack.pop() - 2

    # 60 - int
    if(pc1 == 60):
        pass
        #biosInt(pc2, regList)

    # 0 - halt
    if(pc1 == 0):
        exit()

    # 255 - nop
    if(pc1 == 255):
        pass

while(True):
    if(Core_CPU_rejestrs["pc"] % 2 == 0):
        print(Core_CPU_rejestrs)

    fetch(Core_cpu_ramSpace[Core_CPU_rejestrs["pc"]], Core_cpu_ramSpace[Core_CPU_rejestrs["pc"]+1])
    Core_CPU_rejestrs["pc"] = Core_CPU_rejestrs["pc"] + 2
