# System-Wide_FD_Table
This is a tool to display the tables used by the OS to keep track of open files, assignation of File Descriptors (FD) and processess of the current user.

The program should accept several command line arguments:
- Flagged Arguments
  - --per-process, indicates that only the process FD table will be displayed. Includes the line #, PID of files, and FD of files.

  - --systemWide, indicates that only the system-wide FD table will be displayed. Includes the line #, PID of files, FD of files, and filename of files.

  - --Vnodes, indicates that the Vnodes FD table will be displayed. Includes the line #, FD of files, and inode of files.

  - --composite, indicates that only the composed table will be displayed. Includes the line #, PID of files, FD of files, FD of files, and filename of files.

  - --threshold=X, where X denotes an integer, indicating that processes which have a number of FD assigned larger than X should be flagged in the output.
For this it will list the PID and number of assigned FDs, e.g. PID (FD). If given a positional argument, it will only check the given PID.

- Positional Argument
  - positional argument: Only one positional argument indicating a particular process id number (PID), if not specified the program will attempt to process all the currently running processes for the user executing the program.

## Documentations of the functions
The overview of the functions are in the actual code itself, as I included the details of the functions
similar to Python docstrings.

## Overcoming Problems
Whenever I ran into a problem, I first tried to find out what the problem was. If it was a simple syntax error,
problem solved! However, when I ran into a complex bug, I tried to find what tools I used, and read the documentations
on the tool. If it was hard to understand, I searched it up online to look for explanations. I played around with the tool
in a scratch c file, and then refactored the code after understanding more about the tool.

Ex: When I tried printing PIDs, I did it with the %ld. I read the documentation, searched around online and found out that it should be %d.

## Running the Program

- First, run "make all". This will compile the program and will output an executable file of name "lab2".
        
- *Aside: for next example, the compiled code will be called prog1.*
- ~ lab2 will only print the composite table.
- ~ lab2 14169 will only print files from PID 14149 IF the file with PID 14169 is owned by the current user. Else, it will only print the heading.
- ~ lab2 --composite is equal to lab2.
- ~ lab2 -composite 14169 --systemWide --composite --threshold=20 --Vnodes will print the process FD table, system-wide FD table, Vnode table, composite table and processes which have a number of FD assigned larger than X with the number of FD out ONLY if file with PID 14169 is owned by current user, in order. Else, it will only print out the heading of the all mentioned flags.
