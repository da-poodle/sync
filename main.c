#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sync.h"
#include "file.h"

void print_usage() {
   printf("USAGE: sync [params] source dest\n\n" 
          "   source : the directory files are copied from.\n"
          "   dest   : the directory files are copied to. \n"
          "\nValid parameters: \n\n"
          "   -q  --no-summary       quiet operation with no output.\n\n"
          "   -nf --no-new-folders   don't create any folders.\n"
          "   -nc --no-new-files     don't copy any missing files.\n"
          "   -nu --no-updated-files don't copy existing files that have updated\n"
          "\n"
          "   -dry --dry-run-only    don't do any changes, only show what will be done\n"
          "   -one --one-way-only    update files in the destination but not the source directory\n" 
          "\n");  
   
   exit(1);  
}

void pferror(const char* message, const char* fname) {
     fprintf(stderr, message, fname); 
     exit(1);
}


void set_default_config(struct SyncContext* cxt) {
  // some stats
  cxt->filesCopied = 0;
  cxt->directoriesCreated = 0;
  cxt->existingFiles = 0;
  cxt->existingDirectories = 0;
  cxt->modifiedFilesCopied = 0;
  
  cxt->op_flags = 0;   
  cxt->op_flags = CFG_COPY_CHANGED | CFG_COPY_NEW | CFG_MAKE_FOLDERS | CFG_PRINT_SUMMARY;   
}

void print_summary(struct SyncContext* cxt) {
  
  if (!(cxt->op_flags & CFG_PRINT_SUMMARY)) return;
  
  if (cxt->modifiedFilesCopied > 0 || cxt->filesCopied > 0 || cxt->directoriesCreated > 0) {
      printf("\n  -- Sync Summary --\n");
      if (cxt->op_flags & (CFG_COPY_NEW | CFG_MAKE_FOLDERS | CFG_COPY_CHANGED)) {
          printf("\nContents Modified\n");  
          FLAGGED(CFG_COPY_NEW, printf("    new file(s) copied: %d\n", cxt->filesCopied));
          FLAGGED(CFG_MAKE_FOLDERS, printf("    new directories   : %d\n", cxt->directoriesCreated))
          FLAGGED(CFG_COPY_CHANGED, printf("    modified file(s)  : %d\n", cxt->modifiedFilesCopied));
      }
      
      printf("\nContents Not Modified\n");
      printf("    unmodified files  : %d\n", cxt->existingFiles);
      printf("    existing dirs     : %d\n", cxt->existingDirectories);
  } else {
      printf("All files are up to date.\n");  
  }
  printf("\n");
}

unsigned int clear_flag(unsigned int current, unsigned int mask) {
    return (current & mask)? current ^ mask : current;     
}

unsigned int set_flag(unsigned int current, unsigned int mask) {
    return (current & mask)? current : current | mask;     
}         

unsigned int is_param(char * arg, char *sh, char *lng) {
    return !strcmp(arg, sh) || !strcmp(arg, lng);          
}

void check_params(struct SyncContext* cxt, int argc, char *argv[]) {
  int i;
  char filesset = 0;
  
  for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') { // is an option
         if (is_param(argv[i], "-q", "--no-summary")) { // quiet
            cxt->op_flags = clear_flag(cxt->op_flags, CFG_PRINT_SUMMARY);                  
         }
         else if (is_param(argv[i], "-nf", "--no-new-folders")) { // don't create new folders
            cxt->op_flags = clear_flag(cxt->op_flags, CFG_MAKE_FOLDERS);                               
         }
         else if (is_param(argv[i], "-nc", "--no-new-files")) { // don't create new files
            cxt->op_flags = clear_flag(cxt->op_flags, CFG_COPY_NEW);                               
         }
         else if (is_param(argv[i], "-nu", "--no-updated-files")) { // don't copy updated files
            cxt->op_flags = clear_flag(cxt->op_flags, CFG_COPY_CHANGED);                               
         } 
         else if (is_param(argv[i], "-dry", "--dry-run-only")) { // don't do any changes
            cxt->op_flags = set_flag(cxt->op_flags, CFG_DRY_RUN_ONLY);
         }
         else if (is_param(argv[i], "-one", "--one-way-only")) {
            cxt->op_flags = set_flag(cxt->op_flags, CFG_ONE_WAY_ONLY);    
         } else {
            fprintf(stderr, "Unknown paramater: %s\n\n", argv[i]);
            print_usage();
            exit(1);      
         }
      } else {
         if (!filesset) { cxt->sourceDir = argv[i]; filesset++; } 
         else if (filesset == 1) { cxt->destDir = argv[i]; filesset++; }
         else {
              print_usage();
              exit(1);              
         }
      }
  } 
  if (filesset != 2) {
     print_usage();
     exit(1);                  
  }
  
  // check that the directories exists
  if (access(cxt->sourceDir, F_OK) == -1) pferror(DIR_NOT_EXIST, cxt->sourceDir);
  if (access(cxt->destDir, F_OK) == -1) pferror(DIR_NOT_EXIST, cxt->destDir);
  
  // check that the files are actually directories
  if (file_or_dir(cxt->sourceDir) != IS_DIR) pferror(NOT_A_DIRECTORY, cxt->sourceDir);
  if (file_or_dir(cxt->destDir) != IS_DIR) pferror(NOT_A_DIRECTORY, cxt->destDir); 
}

int main(int argc, char *argv[])
{
  struct SyncContext cxt;  
    
  set_default_config(&cxt);  
  check_params(&cxt, argc, argv);
           
  if (cxt.op_flags & CFG_PRINT_SUMMARY) {
      printf("Starting sync of %s and %s\n\n", cxt.sourceDir, cxt.destDir);
  }
  if (sync_folder(&cxt, cxt.sourceDir, cxt.destDir)) {
      perror("An unexpected error occured");                         
  };   
    
  print_summary(&cxt);

  // print a message to warn the user that nothing was done if it is a dry run
  if (cxt.op_flags & CFG_DRY_RUN_ONLY) fprintf(stderr, MSG_DRY_RUN_WARNING);                   
  
  return 0;
}
