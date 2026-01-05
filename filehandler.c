#include <stdio.h>
#include <stdio.h>

void saveFile(const char *fileName, uint8_t *data, size_t size){
    FILE *filePTR;
    filePTR = fopen(fileName, "wb");
    if(!filePTR){
        printf("Error: Nie mozna zapisac danych do pliku <%s>\n", fileName);
        return;
    }
    fwrite(data, 1, size, filePTR);
    fclose(filePTR);
}

void loadFile(const char *fileName, uint8_t *table, uint16_t maxSize){
    FILE *filePTR;
    filePTR = fopen(fileName, "rb");
    if(!filePTR){
        printf("Error: Nie mozna odczytac danych z pliku <%s>\n", fileName);
        return;
    }
    fseek(filePTR, 0, SEEK_END);
    int64_t fileSize = ftell(filePTR);
    fseek(filePTR, 0, SEEK_SET);

    if(fileSize > maxSize){
        printf("Error: Plik <%s> jest za duzy\n", fileName);
        fileSize = (uint64_t)maxSize;
    }

    fread(table, 1, fileSize, filePTR);
    fclose(filePTR);
}