#include <stdint.h>
#include <string.h>
#include "../include/commands.h"

int md(char** inputElements, FILE* container, uint64_t *currDirectory, uint64_t* prevDirectory, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, uint64_t* eof, int blockSize) {
    uint64_t currFileEntry = *currDirectory;
    // 5. Write new
    if(-1 == *currDirectory) {
        printf("THERE IS NO OTHER FILE IN THE FS\n");
        fseek(container, *nextFreeFileEntry, SEEK_SET);
        fwrite(inputElements[1], sizeof(char), 20, container);
        uint8_t type = 0;
        fwrite(&type, sizeof(uint8_t), 1, container);
        uint8_t is_deleted = 0;
        fwrite(&is_deleted, sizeof(uint8_t), 1, container);
        *currDirectory = *nextFreeFileEntry;
        currFileEntry = *nextFreeFileEntry;
        uint64_t dummy = -1;
        updateNextFreeFileEntry(nextFreeBlock, nextFreeFileEntry, eof, blockSize, container);
        fseek(container, *currDirectory, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        fwrite(&dummy, sizeof(uint64_t), 1, container);
    }
    else {
        uint64_t prevFileEntry = *currDirectory;
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

void rd(char** inputElements, FILE* container, uint64_t* currDirectory, uint64_t* prevDirectory, uint64_t* nextFreeFileEntry) {
    char* fileName = inputElements[1];
    uint64_t currFileEntry = *currDirectory;
    uint64_t prevFileEntry = -1;
    while (-1 != currFileEntry) {
        fseek(container, currFileEntry, SEEK_SET);
        char fileName[20];
        fread(fileName, sizeof(char), 20, container);
        if(!strcmp(fileName, inputElements[1])) {
            fseek(container, sizeof(uint8_t), SEEK_CUR);
            uint8_t is_deleted = 1;
            fwrite(&is_deleted, sizeof(uint8_t), 1, container);

            // removing current from list of used file entries
            uint64_t nextFileEntry;
            fread(&nextFileEntry, sizeof(uint64_t), 1, container);
            if(-1 != prevFileEntry) {
                fseek(container, prevFileEntry, SEEK_SET);
                fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
                fwrite(&nextFileEntry, sizeof(uint64_t), 1, container);
            }
            else *currDirectory = nextFileEntry;

            // adding current to list of deleted (free to use) file entries
            fseek(container, currFileEntry, SEEK_SET);
            fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
            fwrite(nextFreeFileEntry, sizeof(uint64_t), 1, container);
            *nextFreeFileEntry = currFileEntry;
        }
        else {
            fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
            prevFileEntry = currFileEntry;
            fread(&currFileEntry, sizeof(uint64_t), 1, container);
        }
    }
}