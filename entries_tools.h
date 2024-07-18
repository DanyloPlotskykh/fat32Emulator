#ifndef ENTRIES_TOOLS_H
#define ENTRIES_TOOLS_H

#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DISK_SIZE 20 * 1024 * 1024 // 20 MB disk size
#define SECTOR_SIZE 512
#define ROOT_DIR_CLUSTER 2
#define FAT_ENTRY_SIZE 4

extern FILE *disk_file;
extern BootSector boot_sector;
extern uint32_t fat_begin_offset;
extern uint32_t data_begin_offset;
extern uint32_t current_dir_cluster;
extern char current_path[1024];

void read_boot_sector();
uint32_t find_free_cluster();
void read_directory_entries(uint32_t cluster, DirectoryEntry **entries, int *count);
void write_directory_entries(uint32_t cluster, DirectoryEntry *entries, int count);

#endif // ENTRIES_TOOLS_H
