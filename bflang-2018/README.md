
I use CLion to build this project so I don't know what to do if you don't use clion.
Probably using cmake and this folders CMakeLists.txt is enough.
My dependencies are boost::program_options to parse program options and flex/bison to generate the AST.
Flex and bison are in my standard path so the CMakeLists.txt does not explicitly search for them.

A .sh script is located in examples/ which compiles and runs all example *.bl source files in the folder.
The compiled bytecode is put in examples/dst.
Run the script with
cd examples;
sh compile_and_run_examples.sh

For OSX compiled, up-to-date executables are located in cmake-build-release/, which might even be able to run on
other unix systems, but I am not sure.

My project comes with two executables:
bfc - brainfuck compiler
and
bfi - brainfuck interpreter

bfc is similar to gcc and others. You use -i to specify a list of input files, that get compiled into one executable.
You can specify an output file with the -o option, and you can specify a list of paths in which the compiler will
look for source files.
A -d option will write comments into the output binary. Doing so will make the binary not executable because of the comments.
The --verbose or -v option gives a little bit more information on the current compiling files.
The last interesting options are maybe the --output-intermediate and --output-symbol-table options.
Each of these take a filename as argument and the compiler will output the intermediate program and the whole symbol table
to these files.

bfi is the interpreter which takes a .b file as argument and executes it. Output is made to stdout and input is read
via stdin. The debug and breakpoint options allow for dumping the memory on certain instruction and the --numerical-input/output
options change the behaviour io is done.