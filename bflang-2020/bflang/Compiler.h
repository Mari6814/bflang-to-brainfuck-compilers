//
// Created by Marian Plivelic on 09.04.18.
//

#ifndef BRAINFUCKLANG_COMPILER_H
#define BRAINFUCKLANG_COMPILER_H

#include "brainfuck.h"
#include "Environment.h"

namespace brainfuck::compiler {

    /**
     * Pointer types:
     *
     * A) INTERLACED
     * - mark current stack end
     * - goto 0
     * - while (target--) ptr++;
     * - take value at current ptr to mark
     * - requires interlaced stack (double memory usage)
     *
     * B) STACKFRAME_COUNTING
     * - on function call: pass own stackframe begin to next function
     * - also requires stack depth counting on call (e.g. next->esp = this->esp + sizeof (this function); call next)
     * - while (esp -= sizeof current > 0) { move ptr to prev->esp location }
     *     - after esp <= 0 you know that the target address is in the current function stackframe
     * - go back to stack by interlaced stacks or by remembering all (sizeof current) you reverted
     *
     * C) DISALLOW
     * error on pointer arithmetic
     */
    enum class PointerHandler {
        DISALLOW, INTERLACED, STACKFRAME_COUNTING
    };

    /**
     * Manages function implementations that wait on parent symbol to complete
     */
    class FunctionImplementationStack {
        /** @brief Holds data needed to implement a function */
        struct FunctionImplementation {
            FunctionSymbol *function;
            parsers::BrainfuckLangParser::FunctionDeclarationContext *declaration;
            parsers::BrainfuckLangParser::FunctionImplementationContext *implementation;
        };
        /** @brief map of parent class to function implementation data */
        std::multimap<ClassSymbol*, FunctionImplementation> onComplete;
    public:
        /** @brief Implements all functions with the specified class as parent */
        void complete(ClassSymbol *classSymbol);

        /** @brief Adds a function to the implement to-do list */
        void add(FunctionSymbol *function,
                 parsers::BrainfuckLangParser::FunctionDeclarationContext *decl,
                 parsers::BrainfuckLangParser::FunctionImplementationContext *impl);
    };

    struct CompilerSettings {
        /** @brief Size of pointers */
        size_t pointerSize = 1;
        /** @brief Size of function pointers */
        size_t functionPointerSize = 1;
    };

    /**
     * Compiles all sources divided in compiliation units
     * It keeps track of 'where' the compilation is
     */
    class Compiler {
        /** @brief Stack of currently open files */
        static std::vector<std::string> openFiles;
        /** @brief List of already parsed files (prevents re-compiling) */
        static std::set<std::string> parsedFileList;
        /** @brief List of functions to be implemented */
        static FunctionImplementationStack onComplete;
    public:
        /** @brief list of settings during compiler time */
        static CompilerSettings settings;

        /**
         * @brief Single compilation unit step
         * @param file Path to file for this compilation unit step
         */
        static void compile(const std::string &file);

        /**
         * Delays a function implementation until its parent symbol completes
         * @param function
         * @param decl
         * @param impl
         */
        static void addOnComplete(FunctionSymbol *function,
                                  parsers::BrainfuckLangParser::FunctionDeclarationContext *decl,
                                  parsers::BrainfuckLangParser::FunctionImplementationContext *impl);

        /**
         * Implements a function in the current scope.
         * @param decl Declaration relative to the current scope
         * @param impl Implementation
         */
        static void implementFunction(FunctionSymbol *function,
                                      parsers::BrainfuckLangParser::FunctionDeclarationContext *decl,
                                      parsers::BrainfuckLangParser::FunctionImplementationContext *impl);
        /**
         * Implements a class in the current scope.
         * @param decl
         * @param body
         */
        static void implementClass(ClassSymbol *classSymbol,
                                   parsers::BrainfuckLangParser::ClassImplementationContext *body);

        /** @return Path to current file being parsed */
        static const std::string &currentFile() { return openFiles.back(); }
    };
}

#endif //BRAINFUCKLANG_COMPILER_H
