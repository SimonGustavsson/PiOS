#include "fs/fat32driver.h"
#include "types/string.h"

static int fat32_driver_open()
{
    return -1;
}

static int fat32_driver_close()
{
    return -1;
}

static int fat32_driver_read()
{
    return -1;
}

static int fat32_driver_peek()
{
    return -1;
}

static int fat32_driver_tell()
{
    return -1;
}

static int fat32_driver_seek()
{
    return -1;
}

// We don't register a driver struct anymore - we just register the factory function
//fs_driver* fat32_driver_register(void)
//{
//    fs_driver* fat = (fs_driver*)pcalloc(sizeof(fs_driver), 1);
//
//    fat->name_len = 20;
//    fat->name = (char*)pcalloc(sizeof(char), fat->name_len);
//    my_memcpy(fat->name, "Simple Fat32 driver\0", fat->name_len);
//
//    fat->type = FAT32WithCHS;
//    fat->init = &fat32_driver_initialize;
//    fat->op = &fat32_driver_operation;
//
//    return fat;
//}

int fat32_driver_factory(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info)
{
    char* partEntry = (char*)pInfo;
    unsigned int i;
    for (i = 0; i < 16; i++) {
        printf("%h ", partEntry[i]);
    }
    printf("\n");
    printf("Fat32 driver factory, given type: %d, handled types: %d, %d, %d\n", pInfo->type, FAT32WithCHS, Fat32WithLba3, FAT32XWithLba);

    // Check if we can handle the type
    if (pInfo->type != FAT32WithCHS && pInfo->type != Fat32WithLba3 && pInfo->type != FAT32XWithLba)
        return E_UNSUPPORTED;

    fat32_driver_info* info = (fat32_driver_info*)pcalloc(sizeof(fat32_driver_info), 1);
    // Where do we store the device??
    info->basic.driver = 0; // How do we get a hold of the struct that we declared in Register() ?
    // Check
    // Read VBR(?) at pInfo->lba_begin, store VBR and stuff on the driver_info
    info->vbr = 42;
    *driver_info = info;
   
    return 0;
}

int fat32_driver_operation(fs_op operation, void* arg1, void* arg2)
{
    switch (operation)
    {
    case fs_op_read:
        return fat32_driver_read();
    case fs_op_open:
        return fat32_driver_open();
    case fs_op_close:
        return fat32_driver_close();
    case fs_op_seek:
        return fat32_driver_seek();
    case fs_op_tell:
        return fat32_driver_tell();
    default:
        // Not supported - Probably write?
        return -1;
    }
}
