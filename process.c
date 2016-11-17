#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <utime.h>

#include "sync.h"
#include "file.h"

int process_file(struct SyncContext* cxt, const char* source_base, const char* dest_base, const char* name, char is_new, char reversed) {
    char newSource[MAX_PATH];
    char newDest[MAX_PATH];
    struct stat s, sdest;
    struct utimbuf new_times;
    int fileType;
    int err = 0;
    size_t copied = 0;
    
    create_file_path(source_base, name, newSource);
    create_file_path(dest_base, name, newDest);
    
    if (stat(newSource,&s) == 0)
    {
        if (s.st_mode & S_IFREG) { // is a file
             if (is_new && cxt->op_flags & CFG_COPY_NEW) {
                 
                 print2p(cxt, MSG_COPYING_FILE_A_TO_B, newSource, newDest);                 
                 WETRUN(
                    CHK(copy_file(newSource, newDest, &copied)); 
                 )
                 print1p(cxt, MSG_BYTES_COPIED, copied);
                 cxt->filesCopied++;
                 
             } else if (!is_new && cxt->op_flags & CFG_COPY_CHANGED) {
                 if (stat(newDest, &sdest) == 0) {
                     if (s.st_mtime > sdest.st_mtime) { // modified! 
                        
                        print2p(cxt, MSG_COPYING_MODIFIED_FILE_A_TO_B, newSource, newDest);                 
                        WETRUN (
                           CHK(copy_file(newSource, newDest, &copied)); 
                        )
                        print1p(cxt, MSG_BYTES_COPIED, copied);
                        cxt->modifiedFilesCopied++;            
                        
                        WETRUN (
                            // set the modified time so that we can compare again
                            new_times.actime = sdest.st_atime; 
                            new_times.modtime = s.st_mtime;                           
                            if (utime(newDest, &new_times) < 0) GOERR
                        )
                        
                     } else {
                         cxt->existingFiles++;
                         print1p(cxt, MSG_SKIPPING_FILE, newSource);
                     }
                 } else GOERR  
             }
        } else if (s.st_mode & S_IFDIR) {
             if (is_new && cxt->op_flags & CFG_MAKE_FOLDERS) {  
                 
                 print1p(cxt, MSG_CREATING_NEW_DIR, newDest);
                 WETRUN (
                     #ifdef _WIN32
                         CHK(mkdir(newDest));
                     #endif
                     #ifdef linux
                         CHK(mkdir(newDest, s.st_mode)); // linux needs mode parameter
                     #endif
                 )
                 cxt->directoriesCreated++;
                 
                 WETRUN (
                     if (reversed) {
                         CHK(sync_folder(cxt, newDest, newSource)); 
                     } else {
                         CHK(sync_folder(cxt, newSource, newDest));       
                     } 
                 )
                 
             } else if (!is_new) {
                 cxt->existingDirectories++;      
                 if (reversed) {
                     CHK(sync_folder(cxt, newDest, newSource)); 
                 } else {
                     CHK(sync_folder(cxt, newSource, newDest));       
                 } 
             }
                   
        } else {
             print1p(cxt, INVALID_FILE_TYPE, newSource);
             GOERR      
        }
    } else GOERR     

    error:
    return err;
}

int sync_folder(struct SyncContext* cxt, const char* d1, const char* d2) {
    DIR *dir1, *dir2;
    struct dirent *ent1, *ent2;
    int cmp;
    int err = 0;
    
    if ((dir1 = opendir (d1)) == NULL) pferror(COULD_NOT_OPEN, d1);
    if ((dir2 = opendir (d2)) == NULL) pferror(COULD_NOT_OPEN, d2);

    ent1 = readdir(dir1);
    ent2 = readdir(dir2);
    while (ent1 && ent2) {

        // skip the parent and current dirs
        if (is_parent_dir(ent1)) { ent1 = readdir(dir1); continue; }
        if (is_parent_dir(ent2)) { ent2 = readdir(dir2); continue; }

        cmp = strcmp(ent1->d_name, ent2->d_name); 
        if (!cmp) {
           //printf("same files %s\n", ent1->d_name); 
           CHK(process_file(cxt, d1, d2, ent1->d_name, 0, 0));        
           ent1 = readdir(dir1);
           ent2 = readdir(dir2);
        } else if (cmp < 0) {
           //printf("new file! %s\n", ent1->d_name);
           CHK(process_file(cxt, d1, d2, ent1->d_name, 1, 0));
           ent1 = readdir(dir1);    
        } else {
           // file needs to be copied the other way!
           TWOWAY(
               CHK(process_file(cxt, d2, d1, ent2->d_name, 1, 1));    
           )
           ent2 = readdir(dir2);
        }
    }
    while (ent1) {
        if (!is_parent_dir(ent1)) { 
            //printf("new file! %s\n", ent1->d_name);
            CHK(process_file(cxt, d1, d2, ent1->d_name, 1, 0));      
        }
        ent1 = readdir(dir1);
    }
    while (ent2) {
        if (!is_parent_dir(ent2)) { 
            CHK(process_file(cxt, d2, d1, ent2->d_name, 1, 1));          
        }
        ent2 = readdir(dir2);          
    }
    error:
    
    closedir (dir1);
    closedir (dir2);     
    
    return err;
}
