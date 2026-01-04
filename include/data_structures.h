#include <stdint.h>
#include <stdbool.h>

enum FileType {
    _FILE = 0,
    _DIRECTORY = 1
};

struct FileEntry {
    char name[20];
    enum FileType type;
    bool is_deleted;
    uint64_t offsetOfNextFile;
    uint64_t blocksOffsets[1000];
};

struct FileDataBlock {
    char* data;
    int refCount;
    uint64_t offsetOfNextBlockInTable;
};