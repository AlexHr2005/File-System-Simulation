#include <stdint.h>
#include <string.h>
#include "../include/commands.h"
#include "../include/data_structures.h"

int cd(char** inputElemenents, FILE* container, uint64_t *currDirectoryStart, uint64_t* currDirectoryEntry, bool* isCurrentDirRoot, uint64_t* firstFileEntry) {
    if(!strcmp(inputElemenents[1], "..")) {
        if(-1 != *currDirectoryEntry) {
            fseek(container, *currDirectoryEntry, SEEK_SET);
            fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t)  + 1 * sizeof(uint64_t), SEEK_CUR);
            uint64_t test;
            fread(&test, sizeof(uint64_t), 1, container);
            fread(currDirectoryEntry, sizeof(uint64_t), 1, container);
            if(-1 == *currDirectoryEntry) {
                *currDirectoryStart = *firstFileEntry;
                *isCurrentDirRoot = true;
            }
            else {
                fseek(container, *currDirectoryEntry, SEEK_SET);
                fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
                fread(currDirectoryStart, sizeof(uint64_t), 1, container);
            }
        }
        return 0;
    }
    else if(!strcmp(inputElemenents[1], "\\")) {
        *currDirectoryEntry = -1;
        *currDirectoryStart = *firstFileEntry;
        *isCurrentDirRoot = true;
        return 0;
    }
    uint64_t currFile = *currDirectoryStart;
    while(-1 != currFile) {
        fseek(container, currFile, SEEK_SET);
        char fileName[20];
        fread(fileName, sizeof(char), 20, container);
        if(!strcmp(inputElemenents[1], fileName)) {
            uint8_t type;
            fread(&type, sizeof(uint8_t), 1, container);
            if(_DIRECTORY != type) {
                printf("File '%s' is not directory.\n", inputElemenents[1]);
                return 1;
            }
            fseek(container, sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
            fread(currDirectoryStart, sizeof(uint64_t), 1, container);
            fwrite(currDirectoryEntry, sizeof(uint64_t), 1, container);
            *currDirectoryEntry = currFile;
            *isCurrentDirRoot = false;
            return 0;
        }
        else {
            fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
            fread(&currFile, sizeof(uint64_t), 1, container);
        }
    }
    printf("Directory with name '%s' does not exist.\n", inputElemenents[1]);
    return 2;
}

int md(char** inputElements, FILE* container, uint64_t *currDirectoryStart, uint64_t* prevDirectoryEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, uint64_t* eof, int blockSize) {
    uint64_t currFileEntry = *currDirectoryStart;
    // 5. Write new
    if(-1 == *currDirectoryStart) {
        printf("THERE IS NO OTHER FILE IN THE FS\n");
        fseek(container, *nextFreeFileEntry, SEEK_SET);
        fwrite(inputElements[1], sizeof(char), 20, container);
        uint8_t type = 0;
        fwrite(&type, sizeof(uint8_t), 1, container);
        uint8_t is_deleted = 0;
        fwrite(&is_deleted, sizeof(uint8_t), 1, container);
        *currDirectoryStart = *nextFreeFileEntry;
        currFileEntry = *nextFreeFileEntry;
        uint64_t dummy = -1;
        updateNextFreeFileEntry(nextFreeBlock, nextFreeFileEntry, eof, blockSize, container);
        fseek(container, *currDirectoryStart, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        fwrite(&dummy, sizeof(uint64_t), 1, container);
        fwrite(&dummy, sizeof(uint64_t), 1, container);
    }
    else {
        uint64_t prevFileEntry = *currDirectoryStart;
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
        uint8_t type = 0;
        fwrite(&type, sizeof(uint8_t), 1, container);
        uint8_t is_deleted = 0;
        fwrite(&is_deleted, sizeof(uint8_t), 1, container);
        currFileEntry = *nextFreeFileEntry;
        updateNextFreeFileEntry(nextFreeBlock, nextFreeFileEntry, eof, blockSize, container);
        fseek(container, currFileEntry, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        uint64_t dummy = -1;
        fwrite(&dummy, sizeof(uint64_t), 1, container);
        fwrite(&dummy, sizeof(uint64_t), 1, container);
        fseek(container, prevFileEntry + 20 + 2 * sizeof(uint8_t), SEEK_SET);
        fwrite(&currFileEntry, sizeof(uint64_t), 1, container);
    }
}

int clearDir(FILE* container, uint64_t* currDirectoryStart, uint64_t* currDirectoryEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof) {
    printf("CLEAR DIR\n");
    uint64_t currFile = *currDirectoryStart;
    while(-1 != currFile) {
        printf("%ld\n", currFile);
        fseek(container, currFile, SEEK_SET);
        fseek(container, sizeof(char) * 20, SEEK_CUR);
        uint8_t type;
        printf("QQQQQQQQQQQQQQ\n");
        fread(&type, sizeof(uint8_t), 1, container);
        if(_DIRECTORY == type) {
            fseek(container, sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
            uint64_t firstFileInDir;
            fread(&firstFileInDir, sizeof(uint64_t), 1, container);
            printf("SSSSSSSSSSSSSSSSS\n");
            if(-1 != firstFileInDir) {
                clearDir(container, &firstFileInDir, &currFile, nextFreeFileEntry, nextFreeBlock, blockSize, eof);
            }
        }
        uint64_t curr = currFile;
        printf("PREDI\n");
        removeFileFromContainer(container, currFile, currDirectoryEntry, &currFile, nextFreeFileEntry, nextFreeBlock, blockSize, eof, -1);
        /*fseek(container, curr, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        fread(&currFile, sizeof(uint64_t), 1, container);*/
    }
    printf("CLEAR DIR EXIT\n");
}

void rd(char** inputElements, FILE* container, uint64_t* currDirectoryStart, uint64_t* currDirectoryEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof) {
    printf("RD\n");
    uint64_t currFile = *currDirectoryStart;
    while(-1 != currFile) {
        fseek(container, currFile, SEEK_SET);
        char fileName[20];
        fread(fileName, sizeof(char), 20, container);
        if(!strcmp(fileName, inputElements[1])) {
            //TOTO: Check if the file is actually not a directory
            fseek(container, 2 * sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
            uint64_t firstFileInDir;
            fread(&firstFileInDir, sizeof(uint64_t), 1, container);
            if(-1 != firstFileInDir) {
                clearDir(container, &firstFileInDir, &currFile, nextFreeFileEntry, nextFreeBlock, blockSize, eof);
            }
            rm(container, inputElements, currDirectoryEntry, currDirectoryStart, nextFreeFileEntry, nextFreeBlock, blockSize, eof);
            break;
        }
        else {
            fseek(container, currFile, SEEK_SET);
            fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
            fread(&currFile, sizeof(uint64_t), 1, container);
        }
    }
}