#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "include/commands.h"

const char containerName[] = "file_system.bin";
char commandLineSyntax[] = "/$ ";
int blockSize = -1;
uint64_t offsetNextFreeDataBlock = -1;

void loadMetadata(FILE* container) {
    if(-1 == blockSize) {
        fseek(container, 0, SEEK_SET);
        fread(&blockSize, sizeof(int), 1, container);
    }
    if(-1 == offsetNextFreeDataBlock) {
        fseek(container, 8, SEEK_SET);
        fread(&offsetNextFreeDataBlock, sizeof(uint64_t), 1, container);
    }
}

// creates the binary file, gets the size of file content blocks in KB
// and writes it in the contaner metadata
void initializeContainer(FILE** container) {
    *container = fopen(containerName, "wb+");

    printf("Each file will be stored in blocks with defined size. Enter the size (KB):\n");
    scanf("%d", &blockSize);

    fwrite(&blockSize, sizeof(int), 1, *container);

    uint64_t offsetFromStart = sizeof(int) + 3 * sizeof(uint64_t) + 100 * sizeof(uint64_t); // the offset of next av. place for storing file entry
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
    printf("%d\n", blockSize);
    char input[100];
    while (true)
    {
        printf("%s", commandLineSyntax);
        fgets(input, sizeof(input), stdin);
        
        char** inputElements = (char**) malloc(3 * sizeof(char*));
        inputElements[0] = (char*) malloc(6 * sizeof(char));
        int indexOfCurrElement = 0;
        char* currElement = inputElements[indexOfCurrElement];
        int currWriteIndex = 0;
        int inputReadIndex = 0;

        char c;
        while((c = input[inputReadIndex]) != '\n') {
            // TODO: split the input into command and it's parameters
            c = input[inputReadIndex];
            if(c == ' ') {
                currElement[currWriteIndex] = '\0';
                indexOfCurrElement++;
                inputElements[indexOfCurrElement] = (char*) malloc(47 * sizeof(char));
                currElement = inputElements[indexOfCurrElement];
                currWriteIndex = 0;
                inputReadIndex++;
                continue;
            }
            currElement[currWriteIndex] = c;
            currWriteIndex++;
            inputReadIndex++;
        }
        currElement[currWriteIndex] = '\0';

        if(!strcmp(inputElements[0], "cpin")) {
            cpin(inputElements, container, blockSize, &offsetNextFreeDataBlock);
        }
    }

    // the file needs to be flushed every time a writing operation(command) has been performed
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