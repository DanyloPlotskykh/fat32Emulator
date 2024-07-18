#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "entries_tools.h"

void master_boot_record(const char *filename);
void format_disk(const char *filename);
void list_directory();
void create_file(char *filename);
void make_directory(char *dirname);
void change_directory(char *dirname); 

#endif // SYSCALLS_H