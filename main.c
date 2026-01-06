#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "include/commands.h"

const char containerName[] = "file_system.bin";
char commandLineSyntax[] = "/$ ";
int blockSize = -1;
uint64_t firstFileEntry = -1;
uint64_t nextFreeFileEntry = -1;
uint64_t nextFreeBlock = -1;
uint64_t eof = -1;

void loadMetadata(FILE* container) {
    if(-1 == blockSize) {
        fseek(container, 0, SEEK_SET);
        fread(&blockSize, sizeof(int), 1, container);
    }
    if(-1 == firstFileEntry) {
        fseek(container, 4, SEEK_SET);
        fread(&firstFileEntry, sizeof(uint64_t), 1, container);
    }
    if(-1 == nextFreeFileEntry) {
        fseek(container, 12, SEEK_SET);
        fread(&nextFreeFileEntry, sizeof(uint64_t), 1, container);
    }
    if(-1 == nextFreeBlock) {
        fseek(container, 20, SEEK_SET);
        fread(&nextFreeBlock, sizeof(uint64_t), 1, container);
    }
    if(-1 == eof) {
        fseek(container, 28, SEEK_SET);
        fread(&eof, sizeof(uint64_t), 1, container);
    }
}

void writeMetadata(FILE* container) {
    fseek(container, 0, SEEK_SET);
    fwrite(&blockSize, sizeof(int), 1, container);

    fwrite(&firstFileEntry, sizeof(uint64_t), 1, container);

    //fseek(container, 4, SEEK_SET);
    fwrite(&nextFreeFileEntry, sizeof(uint64_t), 1, container);

    //fseek(container, 12, SEEK_SET);
    fwrite(&nextFreeBlock, sizeof(uint64_t), 1, container);

    //fseek(container, 20, SEEK_SET);
    fwrite(&eof, sizeof(uint64_t), 1, container);

    fflush(container);

    loadMetadata(container);
    printf("UPDATE: %d %ld %ld %ld\n", blockSize, nextFreeFileEntry, nextFreeBlock, eof);

}

// creates the binary file, gets the size of file content blocks in KB
// and writes it in the contaner metadata
void initializeContainer(FILE** container) {
    *container = fopen(containerName, "wb+");

    printf("Each file will be stored in blocks with defined size. Enter the size (KB):\n");
    scanf("%d", &blockSize);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);

    fwrite(&blockSize, sizeof(int), 1, *container);

    uint64_t offsetFromStart = -1; // offset of first file entry
    fwrite(&offsetFromStart, sizeof(uint64_t), 1, *container);
    offsetFromStart = sizeof(int) + 4 * sizeof(uint64_t) + 100 * sizeof(uint64_t); // the offset of next av. place for storing file entry
    fwrite(&offsetFromStart, sizeof(uint64_t), 1, *container);
    // the offset of next av. place for storing file data block
    fwrite(&offsetFromStart, sizeof(uint64_t), 1, *container);
    // current end offset (EOF)
    fwrite(&offsetFromStart, sizeof(uint64_t), 1, *container);

    offsetFromStart = -1; // default value for offset of first element in bucket in hash table

    for(int i = 0; i < 100; i++) {
        fwrite(&offsetFromStart, sizeof(uint64_t), 1, *container);
    }

    fflush(*container);
}

void runContainer(FILE* container) {
    loadMetadata(container);
    char input[100];
    while (true)
    {
        printf("%s", commandLineSyntax);
        fgets(input, sizeof(input), stdin);
        
        char** inputElements = (char**) malloc(3 * sizeof(char*));
        inputElements[0] = (char*) malloc(100 * sizeof(char));
        int indexOfCurrElement = 0;
        char* currElement = inputElements[indexOfCurrElement];
        int currWriteIndex = 0;
        int inputReadIndex = 0;

        char c;
        int errors = false;
        while((c = input[inputReadIndex]) != '\n') {
            if(currWriteIndex == 99) {
                printf("Too long argument.\n");
                errors = true;
                break;
            }
            // TODO: split the input into command and it's parameters
            if(c == ' ') {
                currElement[currWriteIndex] = '\0';
                
                if (indexOfCurrElement == 2) {
                    printf("Unsupported command.\n");
                    errors = true;
                    break;
                }

                indexOfCurrElement++;
                inputElements[indexOfCurrElement] = (char*) malloc(100 * sizeof(char));
                currElement = inputElements[indexOfCurrElement];
                currWriteIndex = 0;
                inputReadIndex++;
                continue;
            }
            currElement[currWriteIndex] = c;
            currWriteIndex++;
            inputReadIndex++;
        }
        if(errors) {
            break;
        }
        currElement[currWriteIndex] = '\0';

        if(!strcmp(inputElements[0], "cpin")) {
            if(-1 == cpin(inputElements, container, blockSize, &firstFileEntry, &nextFreeBlock, &nextFreeFileEntry, &eof)) {
                //return;
            }
        }
        else if(!strcmp(inputElements[0], "ls")) {
            ls(container, &firstFileEntry);
        }
        else if(!strcmp(inputElements[0], "cat")) {
            cat(container, inputElements, &firstFileEntry, blockSize);
        }
        writeMetadata(container);
        fflush(container);
    }
}

int main() {
    FILE *container = fopen(containerName, "rb+");
    if(NULL == container) {
        initializeContainer(&container);
    }
    runContainer(container);
    fclose(container);
    return 0;
}