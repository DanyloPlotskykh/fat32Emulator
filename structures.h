#include <stdint.h>

typedef struct {
    char filename[12];
    uint8_t attributes;
    uint32_t first_cluster;
    uint32_t size;
} DirectoryEntry;

typedef struct {
    uint32_t signature;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_count;
    uint32_t total_sectors;
    uint32_t fat_size_sectors;
    uint32_t root_cluster;
} BootSector;