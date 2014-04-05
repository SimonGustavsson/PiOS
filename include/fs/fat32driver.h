#include "memory.h"
#include "fs/fs.h"
#include "util/utilities.h"

typedef struct {
    fs_driver_info basic;

    unsigned int vbr; // This is really a struct, TODO
} fat32_driver_info;

// Public functions
//fs_driver* fat32_driver_register(void);
int fat32_driver_factory(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info);
int fat32_driver_operation(fs_op operation, void* arg1, void* arg2);
