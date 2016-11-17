#ifndef _FILE_H_
#define _FILE_H_

#include <dirent.h>
#include <stdlib.h>

// config
#define FILE_BUFFER_SIZE 4096

#define MAX_PATH_REACHED "ERROR: the path is too long: %s\n"

#define IS_DIR 0
#define IS_FILE 1

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#endif

#ifdef linux
#define PATH_SEPARATOR '/'
#endif

int file_or_dir(const char* path);
int is_parent_dir(struct dirent *ent);
int copy_file(const char * source, const char * dest, size_t* copied);
void create_file_path(const char* base, const char* filename, char* storage);

#endif
