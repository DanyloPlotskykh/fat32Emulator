#include "syscalls.h"

static char *disk_name;
extern FILE *disk_file;
extern BootSector boot_sector;
extern uint32_t fat_begin_offset;
extern uint32_t data_begin_offset;
extern uint32_t current_dir_cluster;
extern char current_path[1024];



static void initialize_disk_file(const char *filename) {
    disk_file = fopen(filename, "rb+");
    if (!disk_file) {
        printf("\033[0;32mDisk file not found. Creating a new one.\033[0m\n");
        disk_file = fopen(filename, "wb+");
        if (!disk_file) {
            perror("Error creating disk file");
            exit(1);
        }
        // Initialize the disk file with zeros
        fseek(disk_file, DISK_SIZE - 1, SEEK_SET);
        fputc(0, disk_file);
        format_disk(filename);
    }
}

void master_boot_record(const char *filename)
{
    initialize_disk_file(filename);
}

void format_disk(const char *filename) {
    // Initialize BootSector
    memset(&boot_sector, 0, sizeof(BootSector));
    boot_sector.bytes_per_sector = SECTOR_SIZE;
    boot_sector.sectors_per_cluster = 1;
    boot_sector.reserved_sector_count = 1;
    boot_sector.fat_count = 1;
    boot_sector.fat_size_sectors = (DISK_SIZE / SECTOR_SIZE / SECTOR_SIZE); // Rough estimate
    boot_sector.total_sectors = DISK_SIZE / SECTOR_SIZE;
    boot_sector.signature = 0xAA550000;

    fseek(disk_file, 0, SEEK_SET);
    fwrite(&boot_sector, sizeof(BootSector), 1, disk_file);

    // Initialize FAT
    fseek(disk_file, SECTOR_SIZE, SEEK_SET); // FAT starts after reserved sectors
    uint32_t fat[boot_sector.fat_size_sectors * SECTOR_SIZE / FAT_ENTRY_SIZE];
    memset(fat, 0, sizeof(fat));
    fat[0] = 0x0FFFFFF8; // End of chain marker for FAT[0]
    fat[1] = 0xFFFFFFFF; // End of chain marker for FAT[1]
    fwrite(fat, sizeof(fat), 1, disk_file);

    // Initialize root directory cluster
    fseek(disk_file, SECTOR_SIZE * (1 + boot_sector.fat_size_sectors), SEEK_SET);
    DirectoryEntry root_dir[SECTOR_SIZE / sizeof(DirectoryEntry)];
    memset(root_dir, 0, sizeof(root_dir));
    fwrite(root_dir, sizeof(root_dir), 1, disk_file);

    printf("Disk formatted successfully.\n");
}

void list_directory() {
    DirectoryEntry *entries;
    int entry_count;

    read_directory_entries(current_dir_cluster, &entries, &entry_count);

    for (int i = 0; i < entry_count; ++i) {
        printf("%s ", entries[i].filename);
    }
    printf("\n");

    free(entries);
}

void create_file(char *filename) {
    DirectoryEntry *entries;
    int entry_count;

    read_directory_entries(current_dir_cluster, &entries, &entry_count);

    // Check if file already exists
    for (int i = 0; i < entry_count; ++i) {
        if (strcmp(entries[i].filename, filename) == 0 && entries[i].attributes != 0x10) {
            fprintf(stderr, "Error: File '%s' already exists\n", filename);
            free(entries);
            return;
        }
    }

    uint32_t new_cluster = find_free_cluster();

    DirectoryEntry new_entry;
    strcpy(new_entry.filename, filename);
    new_entry.attributes = 0x00; // Normal file attribute
    new_entry.first_cluster = new_cluster;
    new_entry.size = 0;

    entries = realloc(entries, (entry_count + 1) * sizeof(DirectoryEntry));
    entries[entry_count++] = new_entry;

    write_directory_entries(current_dir_cluster, entries, entry_count);

    free(entries);
}

void make_directory(char *dirname) {
    DirectoryEntry *entries;
    int entry_count;

    read_directory_entries(current_dir_cluster, &entries, &entry_count);

    // Check if directory already exists
    for (int i = 0; i < entry_count; ++i) {
        if (strcmp(entries[i].filename, dirname) == 0 && entries[i].attributes == 0x10) {
            fprintf(stderr, "Error: Directory '%s' already exists\n", dirname);
            free(entries);
            return;
        }
    }

    uint32_t new_cluster = find_free_cluster();

    DirectoryEntry new_entry;
    strcpy(new_entry.filename, dirname);
    new_entry.attributes = 0x10; // Directory attribute
    new_entry.first_cluster = new_cluster;
    new_entry.size = 0;

    entries = realloc(entries, (entry_count + 1) * sizeof(DirectoryEntry));
    entries[entry_count++] = new_entry;

    write_directory_entries(current_dir_cluster, entries, entry_count);

    // Initialize the new directory with "." and ".." entries
    DirectoryEntry dot_entry;
    strcpy(dot_entry.filename, ".");
    dot_entry.attributes = 0x10; // Directory attribute
    dot_entry.first_cluster = new_cluster;
    dot_entry.size = 0;

    DirectoryEntry dotdot_entry;
    strcpy(dotdot_entry.filename, "..");
    dotdot_entry.attributes = 0x10; // Directory attribute
    dotdot_entry.first_cluster = current_dir_cluster;
    dotdot_entry.size = 0;

    DirectoryEntry new_dir_entries[2] = {dot_entry, dotdot_entry};
    write_directory_entries(new_cluster, new_dir_entries, 2);

    free(entries);
}

void change_directory(char *dirname) {
    DirectoryEntry *entries;
    int entry_count;

    read_directory_entries(current_dir_cluster, &entries, &entry_count);

    if (strcmp(dirname, "..") == 0) {
        if (strcmp(current_path, "/root") == 0) {
            return; // Already at root, do nothing
        }

        // Remove the last part of the path
        char *last_slash = strrchr(current_path, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
        }

        // Change to the parent directory
        for (int i = 0; i < entry_count; ++i) {
            if (strcmp(entries[i].filename, "..") == 0) {
                current_dir_cluster = entries[i].first_cluster;
                break;
            }
        }
    } else {
        // Find the directory entry with the given name
        bool found = false;
        for (int i = 0; i < entry_count; ++i) {
            if (strcmp(entries[i].filename, dirname) == 0 && entries[i].attributes == 0x10) {
                current_dir_cluster = entries[i].first_cluster;
                found = true;
                break;
            }
        }

        if (found) {
            // Append the directory name to the current path
            strcat(current_path, "/");
            strcat(current_path, dirname);
        } else {
            fprintf(stderr, "Error: Directory '%s' not found\n", dirname);
        }
    }

    free(entries);
}
