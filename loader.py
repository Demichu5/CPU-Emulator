def loaderBinFile(fileName, array, index = 0):
    with open(fileName, "r") as file:
        for line in file:
            for opcode in line.split():
                #print(f"index: {i+index}")
                #print(opcode)
                array.insert(index, int(opcode))
                index=index+1
    
    return array
