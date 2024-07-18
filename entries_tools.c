#include "entries_tools.h"

FILE *disk_file = NULL;
BootSector boot_sector;
uint32_t fat_begin_offset;
uint32_t data_begin_offset;
uint32_t current_dir_cluster = ROOT_DIR_CLUSTER;
char current_path[1024] = "/root";

void read_boot_sector() {
    fseek(disk_file, 0, SEEK_SET);
    fread(&boot_sector, sizeof(BootSector), 1, disk_file);

    if (boot_sector.signature != 0xAA550000) {
        fprintf(stderr, "Unknown disk format\n");
        exit(1);
    }

    fat_begin_offset = boot_sector.reserved_sector_count * boot_sector.bytes_per_sector;
    data_begin_offset = fat_begin_offset + (boot_sector.fat_count * boot_sector.fat_size_sectors * boot_sector.bytes_per_sector);
}

static void write_fat_entry(uint32_t cluster, uint32_t value) {
    fseek(disk_file, fat_begin_offset + cluster * FAT_ENTRY_SIZE, SEEK_SET);
    fwrite(&value, FAT_ENTRY_SIZE, 1, disk_file);
}

uint32_t find_free_cluster() {
    printf("LOG - total_clusters = %d\n", boot_sector.total_sectors);
    printf("LOG - sectors_per_cluster = %d\n", boot_sector.sectors_per_cluster);
    uint32_t fat_entries = boot_sector.total_sectors / boot_sector.sectors_per_cluster;
    fseek(disk_file, fat_begin_offset, SEEK_SET);

    for (uint32_t i = 0; i < fat_entries; ++i) {
        uint32_t value;
        fread(&value, FAT_ENTRY_SIZE, 1, disk_file);
        if (value == 0) {
            // Mark as used in FAT
            write_fat_entry(i, 0xFFFFFFFF);
            return i + ROOT_DIR_CLUSTER;
        }
    }

    fprintf(stderr, "Error: No free clusters available\n");
    exit(1);
}

void read_directory_entries(uint32_t cluster, DirectoryEntry **entries, int *count) {
    uint32_t sector_offset = data_begin_offset + (cluster - ROOT_DIR_CLUSTER) * boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    fseek(disk_file, sector_offset, SEEK_SET);

    *entries = NULL;
    *count = 0;

    DirectoryEntry entry;
    while (fread(&entry, sizeof(DirectoryEntry), 1, disk_file) > 0) {
        if (entry.filename[0] == '\0') {
            break; // End of directory entries
        }

        *entries = realloc(*entries, (*count + 1) * sizeof(DirectoryEntry));
        (*entries)[(*count)++] = entry;
    }
}

void write_directory_entries(uint32_t cluster, DirectoryEntry *entries, int count) {
    uint32_t sector_offset = data_begin_offset + (cluster - ROOT_DIR_CLUSTER) * boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    fseek(disk_file, sector_offset, SEEK_SET);

    for (int i = 0; i < count; ++i) {
        fwrite(&entries[i], sizeof(DirectoryEntry), 1, disk_file);
    }
}