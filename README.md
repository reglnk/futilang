# SAMPL
Useless programming language made in ~2 months. Includes:
- miserable syntax
- all interpreter codebase in single C++ file
- assembly-like features
- about 20% of Python 3.10 speed
- OpenGL
- classes
- goto <3
- raw pointers
- memory leaks
- segmentation faults
- and more...

The classes aren't working propery for now. At least it's possible to view OpenGL example until it crashes from defective OOP mechanics =]
I have no time to debug it rn, but nevermind it's not bug it's feature =]]

# Building
1. download the repo in zip file
1. unzip it to `sampl` into your home folder (yeah it best builds on linux)
   ```bash
   export SL_CMD=/bin/rm
   unzip -qo sampl.zip
   $SL_CMD sampl.zip
   ```
2. while not leaving bash after previous commands,
   ```bash
   cd sampl
   $SL_CMD CMakeLists.txt
   cd ..
   $SL_CMD -r sampl
   echo "Done"
   ```
