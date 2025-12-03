#include <stdint.h>
#include <string.h>
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

void updateNextFreeBlock(uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof, int blockSize) {
    if(*nextFreeBlock == *eof) {
        *eof += (blockSize * sizeof(char) + sizeof(int) + sizeof(uint64_t));
        *nextFreeBlock = *eof;

        if(*nextFreeFileEntry == *eof - (blockSize * sizeof(char) + sizeof(int) + sizeof(uint64_t))) {
            *nextFreeFileEntry = *eof;
        }
    }
    else {

    }
}

void writeBlockAtOffset(char data[], int blockSize, int refCount, FILE* container, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof) {
    fseek(container, *nextFreeBlock, SEEK_SET);
    fwrite(data, sizeof(char), blockSize, container);
    fwrite(&refCount, sizeof(int), 1, container);
    uint64_t offset = -1;
    fwrite(&offset, sizeof(uint64_t), 1, container);
    updateNextFreeBlock(nextFreeBlock, nextFreeFileEntry, eof, blockSize);
}

void printHashTable(FILE* container, int blockSize) {
    fseek(container, 28, SEEK_SET);
    for(int i = 0; i < 100; i++) {
        fseek(container, 28 + i * sizeof(uint64_t), SEEK_SET);
        printf("HASH: %d\n", i);
        uint64_t offset;
        fread(&offset, sizeof(uint64_t), 1, container);
        while(-1 != offset) {
            fseek(container, offset, SEEK_SET);
            char data[blockSize + 1];
            fread(data, sizeof(char), blockSize, container);
            data[blockSize] = '\0';
            int refCount;
            fread(&refCount, sizeof(int), 1, container);
            uint64_t nextOffset;
            fread(&nextOffset, sizeof(uint64_t), 1, container);

            printf("Block Offset: %ld\n", offset);
            printf("Data: %s\n", data);
            printf("Ref count: %d\n", refCount);
            printf("Next offset: %ld\n\n", nextOffset);

            offset = nextOffset;
        }
    }
}

int cpin(char** inputElements, FILE* container, int blockSize, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof) {
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
    struct FileEntry fileEntry;
    strcpy(fileEntry.name, inputElements[1]);
    fileEntry.is_deleted = false;

    char buffer[blockSize];
    int hash;
    uint64_t offsetCurrBlock;
    uint64_t offsetLastBlock;
    while(fread(buffer, sizeof(char), blockSize, sourceFile) > 0) {
        printf("block free: %ld, entry free: %ld, eof: %ld\n", *nextFreeBlock, *nextFreeFileEntry, *eof);
        bool blockExists = false;
        offsetCurrBlock = *nextFreeBlock;
        struct FileDataBlock block;
        block.data = buffer;

        hash = getBlockHash(buffer, blockSize);
        printf("hash: %d\n", hash);
        
        uint64_t bucketOffset = sizeof(int) + 3 * sizeof(uint64_t) + hash * sizeof(uint64_t);
        fseek(container, bucketOffset, SEEK_SET);
        uint64_t bucketElement;
        fread(&bucketElement, sizeof(uint64_t), 1, container);
        // 3. Check for each block if it's present in the container already (check the hash table)
        if(-1 == bucketElement) {
            bucketElement = offsetCurrBlock;
            fseek(container, -sizeof(uint64_t), SEEK_CUR);
            fwrite(&bucketElement, sizeof(uint64_t), 1, container);
            block.refCount = 1;
            // 4. Write new blocks
            writeBlockAtOffset(buffer, blockSize, 1, container, nextFreeBlock, nextFreeFileEntry, eof);
            fseek(container, 28, SEEK_SET);
            uint64_t n;
            for(int i = 0; i < 100; i++) {
                fread(&n, sizeof(uint64_t), 1, container);
                printf("%d %ld\n", i, n);
            }
            continue;
        }
        else {
            offsetCurrBlock = bucketElement;
            while (-1 != offsetCurrBlock)
            {
                fseek(container, offsetCurrBlock, SEEK_SET);
                char data[blockSize];
                fread(data, sizeof(char), blockSize, container);
                if(!strcmp(data, buffer)) {
                    fread(&block.refCount, sizeof(int), 1, container);
                    block.refCount++;
                    fseek(container, -sizeof(int), SEEK_CUR);
                    fwrite(&block.refCount, sizeof(int), 1, container);
                    blockExists = true;
                    break;
                }
                offsetLastBlock = offsetCurrBlock;
                printf("%ld\n", offsetLastBlock);
                fseek(container, sizeof(int), SEEK_CUR);
                fread(&offsetCurrBlock, sizeof(uint64_t), 1, container);
            }
        }
        if(!blockExists) {
            offsetCurrBlock = *nextFreeBlock;
            // 4. Write new blocks
            writeBlockAtOffset(buffer, blockSize, 1, container, nextFreeBlock, nextFreeFileEntry, eof);
            fseek(container, offsetLastBlock + blockSize + sizeof(int), SEEK_SET);
            fwrite(&offsetCurrBlock, sizeof(uint64_t), 1, container);
        }
    }
    fflush(container);
    printf("_________________________________________________________\n");
    printHashTable(container, blockSize);

    return 0;
}