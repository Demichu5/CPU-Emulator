#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <stdio.h>
#include <stdint.h>

void saveFile(const char *fileName, uint8_t *data, size_t size);
void loadFile(const char *fileName, uint8_t *table, uint16_t maxSize);

#endif