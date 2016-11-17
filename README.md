# sync
A small command line utility to synchronize files between two folders with many options. 

At present this is configured to compile using Bloodshed C++ IDE however it will be available for linux shortly. 

-- Usage --

Basic usage is as follows:

sync <dir1> <dir2>

By doing this, then:
1. All files and directories from dir1 that do not exist in dir2 are copied. 
2. All files and directories that exist in dir2 that do not exist in dir1 are copied.
3. Any files in dir1 that have a last updated date higher that that of the file in dir2 will be copied.   
4. Any files in dir2 that have a last updated date higher that that of the file in dir1 will be copied. 

There are several options to control what is changed and what is not, get them by typing 'sync' with no parameters.