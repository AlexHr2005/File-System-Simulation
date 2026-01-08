#include <stdio.h>

int cpin(char** inputElements, FILE* container, int blockSize, uint64_t* firstFileEntry, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof);
void ls(FILE* container, uint64_t* firstFileEntry);
void cat(FILE* container, char** inputElements, uint64_t* firstFileEntry, int blockSize);
void rm(FILE* container, char** inputElements, uint64_t* firstFileEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof);
int cpout(char** inputElements, FILE* container, int blockSize, uint64_t* firstFileEntry);
int md(char** inputElements, FILE* container, uint64_t *currentDirectory, uint64_t* prevDirectory, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, uint64_t* eof, int blockSize);
void updateNextFreeFileEntry(uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof, int blockSize, FILE* container);
void rd(char** inputElements, FILE* container, uint64_t* currDirectory, uint64_t* prevDirectory, uint64_t* nextFreeFileEntry);