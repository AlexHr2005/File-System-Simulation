#include <stdint.h>
#include "../include/commands.h"
#include "../include/data_structures.h"

int getBlockHash(char* block, int blockSize) {
    int sum = 0;
    for(int i = 0; i < blockSize; i++) {
        sum += block[i];
    }
    sum %= 100;
    return sum;
}

int cpin(char** inputElements, FILE* container, int blockSize, uint64_t* offsetNextFreeDataBlock) {
    // 4. Write new blocks
    // 5. Write new entry

    // 1. Open new file
    FILE* sourceFile = fopen(inputElements[1], "rb");

    if(NULL == sourceFile) {
        printf(
            "There was a problem opening the file with path %s. Check if it exists.\n",
            inputElements[1] 
        );
        return -1;
    }

    // 2. Get trough the new file, splitting it into blocks along the way
    /*struct FileEntry fileEntry;
    strcpy(fileEntry.name, inputElements[1]);
    fileEntry.is_deleted = false;
    fileEntry.offsetOfFirstBlock = offsetNextFreeDataBlock;*/

    char buffer[blockSize];
    int bytesRead;
    int hash;
    while((bytesRead = fread(buffer, sizeof(char), blockSize, sourceFile)) > 0) {
        struct FileDataBlock block;
        block.data = buffer;

        hash = getBlockHash(buffer, blockSize);
        printf("%d %s\n", hash, buffer);
        /*uint64_t bucketOffset = sizeof(int) + 3 * sizeof(uint64_t) + hash * sizeof(uint64_t);
        fseek(container, bucketOffset, SEEK_SET);
        uint64_t bucketElement;
        fread(&bucketElement, sizeof(uint64_t), 1, container);
        // 3. Check for each block if it's present in the container already (check the hash table)
        if(-1 == bucketElement) {
            bucketElement = offsetNextFreeDataBlock;
            fseek(container, -sizeof(uint64_t), SEEK_CUR);
            fwrite(&bucketElement, sizeof(uint64_t), 1, container);
            block.refCount = 1;
        }*/
    }

    fflush(container);
    return 0;
}