# CSC360 A3
No bonus activities have been completed. 
I've just included a list of possible cases that arised that I have covered (see Resolved Cases) and have not covered (see Unresolved cases).  
For the unresolved cases, I have included a short description of an possible implementation I would do. 

## Resolved Cases
1. What happens when a file name larger than the specified max file name length in Constants.h is inserted?
Diskput outputs an error message to user indicating that the filename exceeds maximum.
Error message: "File name exceeds max length (30)."

2. What happens when an existing file name gets inserted?
Outputs an error message to user indicating that the file already exists.
Error message: "File already exists."

3. What if user passes in a full file path (eg. ../myfile.txt)
Only the base name of the full file path will be saved in the meta data in root directory.
Note: because I have not implemented directory insertion other than the root directory, if the base name of the file to be 
inserted matches an existing file name in the disk then it will trigger a "File already exists." error.
(eg. root directory in disk contains "myfile.txt" and user runs ">>./diskput disk.img ../../myfile.txt" then it will trigger "File already exists.")

4. What if a file size greater than the amount of free space currently available in the disk is being inserted?
File will be rejected with an error message.
Error message: "File exceeds free space available (<amount of free space> bytes)."

## Unresolved cases (tracking issues)

1. What if a directory is inserted?
## Goal
Display fail message.
### Idea to be implemented:
In main, check if the inputted file is a directory. If it is, notify user that this disk doesn't support directory insertions yet.

2. What if a directory is retrieved?
## Goal
Display fail message.
### Idea to be implemented:
After locating the directory to be retrieved, check if the status states it's a directory. If it is, notify user that this disk doesn't support directory retrieval yet.


