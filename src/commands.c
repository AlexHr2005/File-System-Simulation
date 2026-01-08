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

void updateNextFreeBlock(uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof, int blockSize, FILE* container) {
    if(*nextFreeBlock == *eof) {
        *eof += (blockSize * sizeof(char) + sizeof(int) + sizeof(uint64_t));
        *nextFreeBlock = *eof;

        if(*nextFreeFileEntry == *eof - (blockSize * sizeof(char) + sizeof(int) + sizeof(uint64_t))) {
            *nextFreeFileEntry = *eof;
        }
    }
    else {
        fseek(container, *nextFreeBlock, SEEK_SET);
        fseek(container, blockSize * sizeof(char) + sizeof(int), SEEK_CUR);
        uint64_t nextBlock;
        fread(&nextBlock, sizeof(uint64_t), 1, container);
        *nextFreeBlock = nextBlock;
    }
}

void updateNextFreeFileEntry(uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof, int blockSize, FILE* container) {
    if(*nextFreeFileEntry == *eof) {
        *eof += (20 * sizeof(char) + 2 * sizeof(uint8_t) + 1001 * sizeof(uint64_t));
        *nextFreeFileEntry = *eof;

        if(*nextFreeBlock == *eof - (20 * sizeof(char) + 2 * sizeof(uint8_t) + 1001 * sizeof(uint64_t))) {
            *nextFreeBlock = *eof;
        }
    }
    else {
        printf("HERE\n");
        printf("^ %ld\n", *nextFreeFileEntry);
        fseek(container, *nextFreeFileEntry, SEEK_SET);
        fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
        uint64_t nextFreeFile;
        fread(&nextFreeFile, sizeof(uint64_t), 1, container);
        *nextFreeFileEntry = nextFreeFile;
    }
}

void writeBlockAtOffset(char data[], int blockSize, int refCount, FILE* container, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof) {
    fseek(container, *nextFreeBlock, SEEK_SET);
    fwrite(data, sizeof(char), blockSize, container);
    fwrite(&refCount, sizeof(int), 1, container);
    uint64_t currBlock = *nextFreeBlock;
    updateNextFreeBlock(nextFreeBlock, nextFreeFileEntry, eof, blockSize, container);
    fseek(container, currBlock, SEEK_SET);
    fseek(container, blockSize * sizeof(char) + sizeof(int), SEEK_CUR);
    uint64_t offset = -1;
    fwrite(&offset, sizeof(uint64_t), 1, container);
}

void printHashTable(FILE* container, int blockSize) {
    fseek(container, 36, SEEK_SET);
    for(int i = 0; i < 100; i++) {
        fseek(container, 36 + i * sizeof(uint64_t), SEEK_SET);
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

void cat(FILE* container, char** inputElements, uint64_t* firstFileEntry, int blockSize) {
    char* fileName = inputElements[1];
    uint64_t currFileEntry = *firstFileEntry;
    while (-1 != currFileEntry)
    {
        fseek(container, currFileEntry, SEEK_SET);
        char fileName[20];
        fread(fileName, sizeof(char), 20, container);
        fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
        if(!strcmp(fileName, inputElements[1])) {
            int currBlockIndex = 0;
            uint64_t currBlockOffset;
            fseek(container, sizeof(uint64_t), SEEK_CUR);
            fread(&currBlockOffset, sizeof(uint64_t), 1, container);
            while(-1 != currBlockOffset)
            {
                //printf("%ld\n", currBlockOffset);
                fseek(container, currBlockOffset, SEEK_SET);
                char block[blockSize + 1];
                fread(block, sizeof(char), blockSize, container);
                block[blockSize] = '\0';
                printf("%s", block);
                currBlockIndex++;
                fseek(container, currFileEntry + 20 * sizeof(char) + 2 * sizeof(uint8_t) + (currBlockIndex + 1) * sizeof(uint64_t), SEEK_SET);
                fread(&currBlockOffset, sizeof(uint64_t), 1, container);
            }
            printf("\n");
            break;
        }
        else fread(&currFileEntry, sizeof(uint64_t), 1, container);
    }
}

void ls(FILE* container, uint64_t* firstFileEntry) {
    uint64_t currFileEntry = *firstFileEntry;
    while (-1 != currFileEntry)
    {
        fseek(container, currFileEntry, SEEK_SET);
        char fileName[20];
        fread(fileName, sizeof(char), 20, container);
        printf("%s %ld\n", fileName, currFileEntry);
        fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
        fread(&currFileEntry, sizeof(uint64_t), 1, container);
    }
    
}

void removeBlockFromHashTable(FILE* container, int blockSize, uint64_t block) {
    fseek(container, block, SEEK_SET);
    char data[blockSize];
    fread(data, sizeof(char), blockSize, container);
    int hash = getBlockHash(data, blockSize);
    fseek(container, sizeof(int) + 4 * sizeof(uint64_t) + hash * sizeof(uint64_t), SEEK_SET);
    printf("\nhash: %d\nbucket offset: %ld\n", hash, sizeof(int) + 4 * sizeof(uint64_t) + hash * sizeof(uint64_t));
    uint64_t currBlock;
    uint64_t prevBlock = -1;
    fread(&currBlock, sizeof(uint64_t), 1, container);
    printf("curr block: %ld\n", currBlock);
    while(-1 != currBlock) {
        //printf("THERE IS A BLOCK\n");
        char currBlockData[4];
        fseek(container, currBlock, SEEK_SET);
        fread(currBlockData, sizeof(char), blockSize, container);
        if(!strncmp(data, currBlockData, blockSize)) {
            printf("FOUND THE BLOCK\n");
            uint64_t nextBlock;
            fseek(container, sizeof(int), SEEK_CUR);
            fread(&nextBlock, sizeof(uint64_t), 1, container);
            if(-1 == nextBlock && -1 == prevBlock) {
                printf("Only block in bucket\n");
                fseek(container, sizeof(int) + 4 * sizeof(uint64_t) + hash * sizeof(uint64_t), SEEK_SET);
                currBlock = -1;
                fwrite(&currBlock, sizeof(uint64_t), 1, container);
                //printf("Bucket offset: %ld\n", sizeof(int) + 4 * sizeof(uint64_t) + hash * sizeof(uint64_t));
            }
            else {
                if(-1 != prevBlock) {
                    printf("prev: %ld\nnext:%ld\n", prevBlock, nextBlock);
                    fseek(container, prevBlock, SEEK_SET);
                    fseek(container, blockSize * sizeof(char) + sizeof(int), SEEK_CUR);
                    fwrite(&nextBlock, sizeof(uint64_t), 1, container);
                }
                else {
                    fseek(container, sizeof(int) + 4 * sizeof(uint64_t) + hash * sizeof(uint64_t), SEEK_SET);
                    fwrite(&nextBlock, sizeof(uint64_t), 1, container);
                }
            }
            printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
            return;
        }
        else {
            prevBlock = currBlock;
            fseek(container, sizeof(int), SEEK_CUR);
            fread(&currBlock, sizeof(uint64_t), 1, container);
        }
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    }
}

void rm(FILE* container, char** inputElements, uint64_t* firstFileEntry, uint64_t* nextFreeFileEntry, uint64_t* nextFreeBlock, int blockSize, uint64_t* eof) {
    char* fileName = inputElements[1];
    uint64_t currFileEntry = *firstFileEntry;
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
            else *firstFileEntry = nextFileEntry;

            // adding current to list of deleted (free to use) file entries
            fseek(container, currFileEntry, SEEK_SET);
            fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t), SEEK_CUR);
            fwrite(nextFreeFileEntry, sizeof(uint64_t), 1, container);
            *nextFreeFileEntry = currFileEntry;

            // adding current's blocks to the list of free to use blocks, if they are not referenced to by any file
            uint64_t currBlock;
            int blockIndex = 0;
            fread(&currBlock, sizeof(uint64_t), 1, container);
            while(-1 != currBlock) {
                printf("CURRENT BLOCK: %ld", currBlock);
                fseek(container, currBlock, SEEK_SET);
                fseek(container, blockSize * sizeof(char), SEEK_CUR);
                int refCount;
                fread(&refCount, sizeof(int), 1, container);
                refCount--;
                fseek(container, -sizeof(int), SEEK_CUR);
                fwrite(&refCount, sizeof(int), 1, container);
                if(0 == refCount) {
                    removeBlockFromHashTable(container, blockSize, currBlock);
                    fseek(container, currBlock, SEEK_SET);
                    printf("next free block before: %ld\n", *nextFreeBlock);
                    fseek(container, blockSize * sizeof(char) + sizeof(int), SEEK_CUR);
                    fwrite(nextFreeBlock, sizeof(uint64_t), 1, container);
                    *nextFreeBlock = currBlock;
                    printf("next free block after: %ld\n", *nextFreeBlock);
                }
                blockIndex++;
                fseek(container, currFileEntry, SEEK_SET);
                fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t) + (blockIndex + 1) * sizeof(uint64_t), SEEK_CUR);
                fread(&currBlock, sizeof(uint64_t), 1, container);
            }
            break;
        }
        else {
            fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
            prevFileEntry = currFileEntry;
            fread(&currFileEntry, sizeof(uint64_t), 1, container);
        }

    }
    fflush(container);
    //printf("_________________________________________________________\n");
    //printHashTable(container, blockSize);
    printf("List of free blocks:\n");
    fseek(container, *nextFreeBlock, SEEK_SET);
    uint64_t curr = *nextFreeBlock;
    printf("---- %ld\n", *nextFreeBlock);
    printf("EOF %ld\n", *eof);
    if(curr == *eof) {
        return;
    }
    fseek(container, blockSize * sizeof(char) + sizeof(int), SEEK_CUR);
    fread(&curr, sizeof(uint64_t), 1, container);
    while(1) {
        if(curr == *eof) {
        break;
    }
        printf("---- %ld\n", curr);
        fseek(container, curr, SEEK_SET);
        fseek(container, blockSize * sizeof(char) + sizeof(int), SEEK_CUR);
        fread(&curr, sizeof(uint64_t), 1, container);
    }
}

int cpout(char** inputElements, FILE* container, int blockSize, uint64_t* firstFileEntry) {
    FILE* dest = fopen(inputElements[2], "wb");
    printf("%s\n", inputElements[2]);
    if(NULL == dest) {
        printf("File with name %s can not be created.\n", inputElements[2]);
        return -1;
    }

    uint64_t currFile = *firstFileEntry;
    while(-1 != currFile) {
        fseek(container, currFile, SEEK_SET);
        char fileName[20];
        fread(fileName, sizeof(char), 20, container);
        printf("curr file: %s\n", fileName);
        if(!strcmp(fileName, inputElements[1])) {
            char buffer[blockSize];
            int blockIndex = 0;
            fseek(container, 2 * sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
            uint64_t currBlock;
            printf("1\n");
            fread(&currBlock, sizeof(uint64_t), 1, container);
            printf("2\n");
            while(-1 != currBlock) {
                fseek(container, currBlock, SEEK_SET);
                fread(buffer, sizeof(char), blockSize, container);
                printf("curr block: %ld %s\n", currBlock, buffer);
                fwrite(buffer, sizeof(char), blockSize, dest);
                printf("3\n");
                blockIndex++;
                fseek(container, currFile, SEEK_SET);
                fseek(container, 20 * sizeof(char) + 2 * sizeof(uint8_t) + (blockIndex + 1) * sizeof(uint64_t), SEEK_CUR);
                fread(&currBlock, sizeof(uint64_t), 1, container);
            }
            fclose(dest);
            return 0;
        }
        else {
            fseek(container, 2 * sizeof(uint8_t), SEEK_CUR);
            fread(&currFile, sizeof(uint64_t), 1, container);
        }
    }
    printf("File with name %s does not exist.\n", inputElements[1]);
    fclose(dest);
    return 1;
}

int cpin(char** inputElements, FILE* container, int blockSize, uint64_t* firstFileEntry, uint64_t* nextFreeBlock, uint64_t* nextFreeFileEntry, uint64_t* eof) {

    // 1. Open new file
    FILE* sourceFile = fopen(inputElements[1], "rb");

    if(NULL == sourceFile) {
        printf(
            "There was a problem opening the file with path %s. Check if it exists.\n",
            inputElements[1] 
        );
        return -1;
    }
    
    uint64_t currFileEntry = *firstFileEntry;
    // 5. Write new
    if(-1 == *firstFileEntry) {
        printf("THERE IS NO OTHER FILE IN THE FS\n");
        fseek(container, *nextFreeFileEntry, SEEK_SET);
        fwrite(inputElements[1], sizeof(char), 20, container);
        uint8_t type = 1;
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
                printf("File with name '%s' already exists.\n", inputElements[1]);
                fclose(sourceFile);
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

    // 2. Get trough the new file, splitting it into blocks along the way
    struct FileEntry fileEntry;
    strcpy(fileEntry.name, inputElements[1]);
    fileEntry.is_deleted = false;

    char buffer[blockSize];
    int hash;
    uint64_t offsetCurrBlock;
    uint64_t offsetLastBlock;
    int blockIndex = 0;
    while(fread(buffer, sizeof(char), blockSize, sourceFile) > 0) {
        printf("block free: %ld, entry free: %ld, eof: %ld\n", *nextFreeBlock, *nextFreeFileEntry, *eof);
        bool blockExists = false;
        offsetCurrBlock = *nextFreeBlock;
        struct FileDataBlock block;
        block.data = buffer;

        hash = getBlockHash(buffer, blockSize);
        printf("hash: %d\n", hash);
        
        uint64_t bucketOffset = sizeof(int) + 4 * sizeof(uint64_t) + hash * sizeof(uint64_t);
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
            fseek(container, 36, SEEK_SET);
            uint64_t n;
            for(int i = 0; i < 100; i++) {
                fread(&n, sizeof(uint64_t), 1, container);
                printf("%d %ld\n", i, n);
            }
            fseek(container, currFileEntry + 20 * sizeof(char) + 2 * sizeof(uint8_t) + (1 + blockIndex) * sizeof(uint64_t), SEEK_SET);
            printf("OFFSET OF CURR BLOCK TO BE WRITTEN TO THE FILE: %ld\n", offsetCurrBlock);
            fwrite(&offsetCurrBlock, sizeof(uint64_t), 1, container);
            blockIndex++;
            continue;
        }
        else {
            offsetCurrBlock = bucketElement;
            while (-1 != offsetCurrBlock)
            {
                fseek(container, offsetCurrBlock, SEEK_SET);
                char data[blockSize];
                fread(data, sizeof(char), blockSize, container);
                if(!strncmp(data, buffer, blockSize)) {
                    fread(&block.refCount, sizeof(int), 1, container);
                    block.refCount++;
                    fseek(container, -sizeof(int), SEEK_CUR);
                    fwrite(&block.refCount, sizeof(int), 1, container);
                    blockExists = true;
                    break;
                }
                offsetLastBlock = offsetCurrBlock;
                printf("lala %ld\n", offsetLastBlock);
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
        fseek(container, currFileEntry + 20 * sizeof(char) + 2 * sizeof(uint8_t) + (1 + blockIndex) * sizeof(uint64_t), SEEK_SET);
        printf("OFFSET OF CURR BLOCK TO BE WRITTEN TO THE FILE: %ld\n", offsetCurrBlock);
        fwrite(&offsetCurrBlock, sizeof(uint64_t), 1, container);
        blockIndex++;
    }
    uint64_t blocksEnd = -1;
    fwrite(&blocksEnd, sizeof(uint64_t), 1, container);
    fflush(container);
    printf("_________________________________________________________\n");
    printHashTable(container, blockSize);

    uint64_t prevFileEntry;

    return 0;
}