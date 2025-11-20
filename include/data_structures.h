#include <stdint.h>
#include <stdbool.h>

enum FileType {
    _FILE,
    _DIRECTORY
};

struct FileEntry {
    char name[20];
    enum FileType type;
    bool is_deleted;
    uint64_t offsetOfFirstBlock;
    uint64_t offsetOfLastBlock;
    uint64_t offsetOfNextFile;
};

struct FileDataBlock {
    char* data;
    int refCount;
    uint64_t offsetOfNextBlockInFile;
    uint64_t offsetOfNextBlockInTable;
};