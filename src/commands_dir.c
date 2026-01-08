#include <stdint.h>
#include <string.h>
#include "../include/commands.h"

int md(char** inputElements, FILE* container, uint64_t *firstFileEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, uint64_t* eof, int blockSize) {
    uint64_t currFileEntry = *firstFileEntry;
    // 5. Write new
    if(-1 == *firstFileEntry) {
        printf("THERE IS NO OTHER FILE IN THE FS\n");
        fseek(container, *nextFreeFileEntry, SEEK_SET);
        fwrite(inputElements[1], sizeof(char), 20, container);
        uint8_t type = 0;
        fwrite(&type, sizeof(uint8_t), 1, container);
        uint8_t is_deleted = 0;
        fwrite(&is_deleted, sizeof(uint8_t), 1, container);
        *firstFileEntry = *nextFreeFileEntry;
        currFileEntry = *nextFreeFileEntry;
        uint64_t dummy = -1;
        updateNextFreeFileEntry(nextFreeBlock, nextFreeFileEntry, eof, blockSize, container);
        fseek(container, *firstFileEntry, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        fwrite(&dummy, sizeof(uint64_t), 1, container);
    }
    else {
        uint64_t prevFileEntry = *firstFileEntry;
        while(-1 != currFileEntry) {
            char fileName[20];
            fseek(container, currFileEntry, SEEK_SET);
            fread(fileName, sizeof(char), 20, container);       
            if(!strcmp(inputElements[1], fileName)) {
                printf("Directory with name '%s' already exists.\n", inputElements[1]);
                return 1;
            }
            prevFileEntry = currFileEntry;
            fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
            fread(&currFileEntry, sizeof(uint64_t), 1, container);
        }
        fseek(container, *nextFreeFileEntry, SEEK_SET);
        fwrite(inputElements[1], sizeof(char), 20, container);
        uint8_t type = 1;
        fwrite(&type, sizeof(uint8_t), 1, container);
        uint8_t is_deleted = 0;
        fwrite(&is_deleted, sizeof(uint8_t), 1, container);
        currFileEntry = *nextFreeFileEntry;
        updateNextFreeFileEntry(nextFreeBlock, nextFreeFileEntry, eof, blockSize, container);
        fseek(container, currFileEntry, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        uint64_t dummy = -1;
        fwrite(&dummy, sizeof(uint64_t), 1, container);
        fseek(container, prevFileEntry + 20 + 2 * sizeof(uint8_t), SEEK_SET);
        fwrite(&currFileEntry, sizeof(uint64_t), 1, container);
    }
}