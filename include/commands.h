#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int cpin(char** inputElements, FILE* container, int blockSize, uint64_t* currentDirectoryEntry, uint64_t* firstFileEntry, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof);
void ls(FILE* container, uint64_t* firstFileEntry);
void cat(FILE* container, char** inputElements, uint64_t* firstFileEntry, int blockSize);
void rm(FILE* container, char** inputElements, uint64_t* currDirectoryEntry, uint64_t* firstFileEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof);
int cpout(char** inputElements, FILE* container, int blockSize, uint64_t* firstFileEntry);
int md(char** inputElements, FILE* container, uint64_t *currDirectoryStart, uint64_t* currDirectoryEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, uint64_t* eof, int blockSize);
void updateNextFreeFileEntry(uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof, int blockSize, FILE* container);
void rd(char** inputElements, FILE* container, uint64_t* currDirectoryStart, uint64_t* currDirectoryEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof);
int cd(char** inputElemenents, FILE* container, uint64_t *currDirectoryStart, uint64_t* currDirectoryEntry, bool* isCurrDirRoot, uint64_t* firstFileEntry);
void removeFileFromContainer(FILE* container, uint64_t currFileEntry, uint64_t* currDirectoryEntry, uint64_t* firstFileEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof, uint64_t prevFileEntry);