#ifndef _SYNC_H_
#define _SYNC_H_

// file errors mainly
#define DIR_NOT_EXIST "ERROR: The directory %s doesn't exist\n"
#define NOT_A_DIRECTORY "ERROR: %s is not a directory\n"
#define COULD_NOT_OPEN "ERROR: could not open %s for reading\n"
#define STAT_FAILED "ERROR: Can't get info of file %s\n"
#define INVALID_FILE_TYPE "%s is neither a file or dir, wtf?!\n"

// information messages
#define MSG_COPYING_FILE_A_TO_B "Copying %s to %s"
#define MSG_COPYING_MODIFIED_FILE_A_TO_B "Updating file %s to %s"
#define MSG_SKIPPING_FILE "Skipping unchanged file: %s \n"
#define MSG_CREATING_NEW_DIR "New directory %s\n"
#define MSG_BYTES_COPIED " --> %d bytes copied\n"
#define MSG_DRY_RUN_WARNING \
   "WARNING: this is a dry run\n" \
   "   - no files or folders have been changed.\n" \
   "   - new directories have not been checked.\n\n" 

#define CHK(x) if ((err = x) != 0) goto error;
#define GOERR err = -1; goto error;

#define CFG_COPY_CHANGED     0x1
#define CFG_COPY_NEW         0x10
#define CFG_MAKE_FOLDERS     0x100
#define CFG_PRINT_SUMMARY    0x1000
#define CFG_DRY_RUN_ONLY     0x10000
#define CFG_READ_HIT_FILE    0x100000
#define CFG_WRITE_HIT_FILE   0x1000000
#define CFG_ONE_WAY_ONLY     0x10000000

struct SyncContext {
    
    unsigned int filesCopied;
    unsigned int directoriesCreated;
    unsigned int existingFiles;
    unsigned int existingDirectories;
    unsigned int modifiedFilesCopied;     

    unsigned int op_flags;
    
    const char* sourceDir;
    const char* destDir;
};

int sync_folder(struct SyncContext*, const char*, const char*);

#define print1p(cxt, pattern, p1) if (cxt->op_flags & CFG_PRINT_SUMMARY) printf(pattern, p1);
#define print2p(cxt, pattern, p1, p2) if (cxt->op_flags & CFG_PRINT_SUMMARY) printf(pattern, p1, p2);

#define FLAGGED(flag, f) if (cxt->op_flags & flag)  f;

#define WETRUN(f) if (!(cxt->op_flags & CFG_DRY_RUN_ONLY)) { f }
#define TWOWAY(f) if (!(cxt->op_flags & CFG_ONE_WAY_ONLY)) { f }
#endif
