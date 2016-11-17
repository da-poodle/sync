#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

#include "file.h"

int file_or_dir(const char* path) {
    struct stat s;
    if (stat(path,&s) == 0)
    {
        if (s.st_mode & S_IFDIR) return IS_DIR; // directory
        else if (s.st_mode & S_IFREG) return IS_FILE; // file 
    }
    return -1; // error
}

int is_parent_dir(struct dirent *ent) {
    return ent == NULL || !strcmp(".",ent->d_name) || !strcmp("..",ent->d_name);
}

int copy_file(const char * source, const char * dest, size_t* copied)
{   
    int srcfd, destfd, res;
    
    if ((srcfd = open(source, O_RDONLY)) < 0) {
        return errno;
    }
    if ((destfd = open(dest, O_RDWR | O_CREAT | O_APPEND)) < 0) {
        return errno;
    }
    res = copy_file_stream(srcfd, destfd, copied);
    
    close(srcfd);
    close(destfd);

    return res;
}

int copy_file_stream(int source, int dest, size_t* copied) {
    char    c[FILE_BUFFER_SIZE];
    int red;
    
    *copied = 0;
    while ((red = read(source, c, FILE_BUFFER_SIZE)) > 0) {          
        write(dest, c, red);
        *copied += red;      
    }
    
    return red;
}

void create_file_path(const char* base, const char* filename, char* storage) {
     int plen = strlen(base);
     int flen = strlen(filename);
     
     if (plen + flen + 1 > MAX_PATH) {
        pferror(MAX_PATH_REACHED, base);         
     }     
     
     strncpy(storage, base, plen);
     storage += plen;
     *storage++ = PATH_SEPARATOR;
     strncpy(storage, filename, flen);
     storage[flen] = '\0';
}
