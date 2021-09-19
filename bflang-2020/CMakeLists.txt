cmake_minimum_required(VERSION 3.10)
project(BrainfuckLang CXX)
set(CMAKE_CXX_STANDARD 17)

# Location of antlr4 stuff
set(ANTLR_CLASSPATH /usr/local/Cellar/antlr/4.7.1/antlr-4.7.1-complete.jar)
set(ANTLR_LIBS /usr/local/lib/libantlr4-runtime.a)
set(ANTLR_INCLUDE /usr/local/include/antlr4-runtime)
include_directories(${ANTLR_INCLUDE})

# default use source dir in includes
include_directories(${PROJECT_SOURCE_DIR})

# Directory where the grammars are located
set(GRAMMARS_DIR ${PROJECT_SOURCE_DIR}/grammars)
# Target directory where the grammar java/c++ code should be compiled to
set(PARSERS_DIR ${PROJECT_SOURCE_DIR}/parsers)

# Shared sourcefiles between compiler and intermediate assembly language
set(SHARED_SOURCES brainfuck.h util.h)

# Sources for the intermediate/assembly antlr4 parser
set(INTERMEDIATE_PARSER_SOURCES ${PARSERS_DIR}/BrainfuckLangAssemblyBaseListener.cpp ${PARSERS_DIR}/BrainfuckLangAssemblyBaseListener.h ${PARSERS_DIR}/BrainfuckLangAssemblyLexer.cpp ${PARSERS_DIR}/BrainfuckLangAssemblyLexer.h ${PARSERS_DIR}/BrainfuckLangAssemblyListener.cpp ${PARSERS_DIR}/BrainfuckLangAssemblyListener.h ${PARSERS_DIR}/BrainfuckLangAssemblyParser.cpp ${PARSERS_DIR}/BrainfuckLangAssemblyParser.h ${PARSERS_DIR}/BrainfuckLangAssemblyVisitor.cpp ${PARSERS_DIR}/BrainfuckLangAssemblyVisitor.h)
# Files only the intermediate assembly language compiler needs
set(INTERMEDIATE_COMPILER_SOURCES assembly/assembly.h)

# Sources for my brainfuck language antlr4 parser
set(PARSER_SOURCES ${PARSERS_DIR}/BrainfuckLangBaseListener.cpp ${PARSERS_DIR}/BrainfuckLangBaseListener.h ${PARSERS_DIR}/BrainfuckLangLexer.cpp ${PARSERS_DIR}/BrainfuckLangLexer.h ${PARSERS_DIR}/BrainfuckLangListener.cpp ${PARSERS_DIR}/BrainfuckLangListener.h ${PARSERS_DIR}/BrainfuckLangParser.cpp ${PARSERS_DIR}/BrainfuckLangParser.h ${PARSERS_DIR}/BrainfuckLangVisitor.cpp ${PARSERS_DIR}/BrainfuckLangVisitor.h)
# Files only the high language compiler needs
set(COMPILER_SOURCES bflang/Compiler.cpp bflang/Compiler.h bflang/Environment.cpp bflang/Environment.h bflang/Symbol.cpp bflang/Symbol.h Location.h bflang/forward.h util.cpp bflang/SymbolTableBuilder.cpp bflang/SymbolTableBuilder.h CompilerException.cpp CompilerException.h bflang/ClassSymbol.cpp bflang/ClassSymbol.h bflang/VariableSymbol.cpp bflang/VariableSymbol.h bflang/FunctionSymbol.cpp bflang/FunctionSymbol.h bflang/Type.cpp bflang/Type.h bflang/AliasSymbol.cpp bflang/AliasSymbol.h)

# Compiles the g4 grammar for my brainfuck language into java classes and c++ sources
add_custom_command(
        OUTPUT ${PARSER_SOURCES} ${PARSERS_DIR}/BrainfuckLangLexer.class ${PARSERS_DIR}/BrainfuckLangParser.class
        COMMAND antlr4 -visitor -Dlanguage=Cpp ${GRAMMARS_DIR}/BrainfuckLang.g4 -o ${PARSERS_DIR} -package brainfuck::parsers
        COMMAND antlr4 ${GRAMMARS_DIR}/BrainfuckLang.g4 -o ${PARSERS_DIR}
        COMMAND javac -cp ${ANTLR_CLASSPATH} BrainfuckLang*.java
        DEPENDS ${GRAMMARS_DIR}/BrainfuckLang.g4
        COMMENT "Compiling BrainfuckLang grammar"
        WORKING_DIRECTORY ${PARSERS_DIR})

# Compiles the intermediate g4 grammar into java classes and c++ sources
add_custom_command(
        OUTPUT ${INTERMEDIATE_PARSER_SOURCES} ${PARSERS_DIR}/BrainfuckLangAssemblyLexer.class ${PARSERS_DIR}/BrainfuckLangAssemblyParser.class
        COMMAND antlr4 -visitor -Dlanguage=Cpp ${GRAMMARS_DIR}/BrainfuckLangAssembly.g4 -o ${PARSERS_DIR} -package brainfuck::parsers
        COMMAND antlr4 ${GRAMMARS_DIR}/BrainfuckLangAssembly.g4 -o ${PARSERS_DIR}
        COMMAND javac -cp ${ANTLR_CLASSPATH} BrainfuckLangAssembly*.java
        DEPENDS ${GRAMMARS_DIR}/BrainfuckLangAssembly.g4
        COMMENT "Compiling BrainfuckLangAssembly grammar"
        WORKING_DIRECTORY ${PARSERS_DIR})

# Custom target that builds the intermediate and language grammars
add_custom_target(antlr
        DEPENDS ${PARSERS_DIR}/BrainfuckLangLexer.class ${PARSERS_DIR}/BrainfuckLangParser.class ${PARSERS_DIR}/BrainfuckLangAssemblyLexer.class ${PARSERS_DIR}/BrainfuckLangAssemblyParser.class)

# My Brainfuck language compiler
add_executable(bfc bfc.cpp ${SHARED_SOURCES} ${PARSER_SOURCES} ${COMPILER_SOURCES})
# Compiler for the intermediate language my brainfuck language requires
add_executable(bfa bfa.cpp ${SHARED_SOURCES} ${INTERMEDIATE_PARSER_SOURCES} ${INTERMEDIATE_COMPILER_SOURCES})
# An interpreter for the usual brainfuck bytecode
add_executable(bfi bfi.cpp ${SHARED_SOURCES})

# Link antlr4 with the executables
target_link_libraries(bfc ${ANTLR_LIBS})
target_link_libraries(bfa ${ANTLR_LIBS})