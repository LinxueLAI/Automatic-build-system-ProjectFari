# Project: Fari -- a mini automatic build system
  1. Description of the Fari project:
  (Fari is a word in Esperanto which means make in English)
  The project involves writing a simplified version of the make program, which helps automate the compilation of programs.However, Fari has fewer features than its big brother from the GNU software collection: it only automates the compilation of a single C program using the gcc compiler. The syntax for fari is as follows: fari [file-description]. If the name of the description file is omitted, default will be used. If the specified file does not exist, fari ends with an error code and a message.
  
  2. Fari's execution sequence
  Fari compiles all the .c files into .o files (using gcc -c) and then links to all the .o files in the final executable. Like make, fari recompiles only what is needed. The following algorithm is used to decide what should be recompiled: 
  - For each .c file: If there is no .o file does match, it must recompile the .c (with gcc -c, and adding the specified flags).
      - If there is a .o file but the .c is more recent, it must also recompile the .c (in the same way). 
      - If there is a newer header file (.h) than the .o file, it must recompile the .c file. It must recompile all source files when a header file is edited, even if that header is not used.
      - If the executable exists and is more recent than all the .o files, there is no need to start linking again. Otherwise, use gcc -o by adding the flags and the specified libraries. Of course, if a specified .c or .h ﬁ le does not exist, fari must end with an error code and a message. If gcc indicates compilation errors, fari must also end with an error message. In the case where fari is executed correctly, that the various phases of the compilation execute correctly and that the executable has been saved, the program must return code 0 and only in this case.
   3. Possible extensions
   - JSON log file: We can produce a log file in JSON format named logs.json. Its format is fixed and we need to respect it since this file will be used by automatic tests. This file is alse useful for debugging the execution of our program.
   - Compilation of Java: if the file contains lines J <file>, fari compiles them as java source files (with the javac program). We can consider that a fascist can only contain the instructions to compile C or compile Java but not both at the same time.
   - Continuation on error: when the -k flag is passed to the fari program, it does not stop immediately if one of the child processes returns an error code. Instead, it issues any remaining commands that do not depend on this thread, and returns an error code only after completion.
   - Headers linked to source files: The purpose of this extension is to add a new format for the CH description file which will link one or more header files to one or more source files. Thus the recompilation will have to be more selective: we will only recompile the source files, one of the linked header files of which has been modified.
   - Globbing: we want to be able to use the character * wherever we have to list files.

