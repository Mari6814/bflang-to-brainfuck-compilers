//
// Created by Marian Plivelic on 09.04.18.
//

#include <fstream>
#include <chrono>
#include "Compiler.h"
#include "Symbol.h"
#include "SymbolTableBuilder.h"
#include "ClassSymbol.h"
#include "FunctionSymbol.h"
#include "VariableSymbol.h"
#include "CompilerException.h"

#define START_TIMER(name) auto (name) = chrono::high_resolution_clock::now()
#define STOP_TIMER(name) auto name ## Delta = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - (name)).count()

using namespace antlr4;
using namespace brainfuck::parsers;
using namespace std;
namespace brainfuck::compiler {
    /**
     * static variables
     */
    std::vector<std::string> Compiler::openFiles;
    std::set<std::string> Compiler::parsedFileList;
    FunctionImplementationStack Compiler::onComplete;
    CompilerSettings Compiler::settings;

    void Compiler::compile(const std::string &file) {
        // Remember to not parse same file twice
        auto alreadyParsed = !parsedFileList.insert(file).second;
        if (alreadyParsed)
            return;

        // remember which file the current main context refers to
        openFiles.push_back(file);

        // start measuring full compile process
        START_TIMER(fullTime);

        // parse the file
        START_TIMER(parseTime);
        util::parser::ParserUtil<parsers::BrainfuckLangLexer, parsers::BrainfuckLangParser> pu;
        auto *ctx = pu.parse(file.c_str());
        STOP_TIMER(parseTime);
        cout << "Parsed '" << file << "' in " << parseTimeDelta << "ms" << endl;

        // compile imported files
        //for (auto imported : ctx->imports())
        //    compile(imported->string()->getText());

        // compile ctx
        START_TIMER(compileTime);
        SymbolTableBuilder().visit(ctx);
        STOP_TIMER(compileTime);
        STOP_TIMER(fullTime);
        cout << "Compiled '" << file << "' in " << compileTimeDelta << "ms (cumulative " << fullTimeDelta << "ms)" << endl;

        openFiles.pop_back();
    }

    void Compiler::implementClass(ClassSymbol *classSymbol,
                                  parsers::BrainfuckLangParser::ClassImplementationContext *body) {
        assert(body != nullptr);

        // check for multiple implementations
        if(classSymbol->isComplete) {
            exceptions::emit(body, "Multiple definitions found for " + classSymbol->qualifiedId() +
                                   " defined at: " + std::string(classSymbol->implementationLocation));
        }

        SymbolTable::push(classSymbol);

        // parse class body symbols
        SymbolTableBuilder().visit(body);

        // add implementation information
        classSymbol->setCompleted(body);

        // implement onComplete functions
        onComplete.complete(classSymbol);

        SymbolTable::pop();
    }

    void Compiler::implementFunction(FunctionSymbol *function,
                                     parsers::BrainfuckLangParser::FunctionDeclarationContext *decl,
                                     parsers::BrainfuckLangParser::FunctionImplementationContext *impl) {
        assert(impl != nullptr);

        // check not previously implemented
        if (function->isComplete)
            exceptions::emit(impl, "Redefinition of function " + function->qualifiedId() + " defined at " +
                                   std::string(function->as<FunctionSymbol>()->implementationLocation));

        // check not interface
        if (function->parent->is<ClassSymbol>() && function->parent->as<ClassSymbol>()->isInterface)
            exceptions::emit(impl, "Interface functions may not be implemented, only overriden");

        // mark implementation location
        function->setCompleted(impl);

        std::cout << function->implementationLocation << "\timplement\t" << function->qualifiedId() << std::endl;

        // parse body
        SymbolTable::push(function);
        SymbolTableBuilder().visit(decl);
        SymbolTableBuilder().visit(impl);
        SymbolTable::pop();
    }

    void
    Compiler::addOnComplete(FunctionSymbol *function, parsers::BrainfuckLangParser::FunctionDeclarationContext *decl,
                            parsers::BrainfuckLangParser::FunctionImplementationContext *impl) {
        onComplete.add(function, decl, impl);
    }

    void FunctionImplementationStack::add(FunctionSymbol *function,
                                          parsers::BrainfuckLangParser::FunctionDeclarationContext *decl,
                                          parsers::BrainfuckLangParser::FunctionImplementationContext *impl) {
        std::pair<ClassSymbol *, FunctionImplementation> value;
        value.first = function->parent->as<ClassSymbol>();
        value.second.function = function;
        value.second.declaration = decl;
        value.second.implementation = impl ;
        onComplete.insert(value);
    }

    void FunctionImplementationStack::complete(ClassSymbol *classSymbol) {
        // get matches
        auto range = onComplete.equal_range(classSymbol);

        // copy matches
        std::vector<FunctionImplementation> impls;
        impls.reserve(static_cast<unsigned long>(std::distance(range.first, range.second)));
        for (auto it = range.first; it != range.second; ++it)
            impls.push_back(it->second);

        // delete class reference
        onComplete.erase(classSymbol);

        SymbolTable::push(classSymbol);
        for (auto &impl : impls)
            Compiler::implementFunction(impl.function, impl.declaration, impl.implementation);
        SymbolTable::pop();
    }
}