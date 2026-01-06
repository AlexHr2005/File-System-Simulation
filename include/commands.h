#include <stdio.h>

int cpin(char** inputElements, FILE* container, int blockSize, uint64_t* firstFileEntry, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof);
void ls(FILE* container, uint64_t* firstFileEntry);
void cat(FILE* container, char** inputElements, uint64_t* firstFileEntry, int blockSize);