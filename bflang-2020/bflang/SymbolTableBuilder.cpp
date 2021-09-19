//
// Created by Marian Plivelic on 24.04.18.
//

#include "SymbolTableBuilder.h"
#include "bflang/Symbol.h"
#include "Environment.h"
#include "ClassSymbol.h"
#include "FunctionSymbol.h"
#include "VariableSymbol.h"
#include "Compiler.h"
#include "Type.h"
#include "AliasSymbol.h"

namespace brainfuck::compiler {

    antlrcpp::Any brainfuck::compiler::SymbolTableBuilder::visitFunctionDeclarationStatement(
            parsers::BrainfuckLangParser::FunctionDeclarationStatementContext *ctx) {
        // declare function
        auto symbol = SymbolTable::declare(ctx->functionDeclaration(), ctx->functionImplementation()).resolvedSymbol;

        std::cout << symbol->firstDeclaration() << "\tfunction\t" << symbol->qualifiedId() << std::endl;

        // implement or delay
        if (ctx->functionImplementation() != nullptr) {
            if (symbol->parent->is<ClassSymbol>() && SymbolTable::getTop() == symbol->parent){
                Compiler::addOnComplete(symbol->as<FunctionSymbol>(), ctx->functionDeclaration(),
                                        ctx->functionImplementation());
            } else {
                // else implement immediately
                Compiler::implementFunction(symbol->as<FunctionSymbol>(), ctx->functionDeclaration(),
                                            ctx->functionImplementation());
            }
        }

        return symbol;
    }

    antlrcpp::Any brainfuck::compiler::SymbolTableBuilder::visitClassDeclarationTypeName(
            parsers::BrainfuckLangParser::ClassDeclarationTypeNameContext *ctx) {
        // declare class
        Symbol *symbol = SymbolTable::declare(ctx->classDeclaration(), ctx->classImplementation()).resolvedSymbol;

        std::cout << symbol->firstDeclaration() << "\tclass\t\t" << symbol->qualifiedId() << std::endl;

        // declare class body symbols
        if (ctx->classImplementation())
            Compiler::implementClass(symbol->as<ClassSymbol>(), ctx->classImplementation());

        return symbol;
    }

    antlrcpp::Any brainfuck::compiler::SymbolTableBuilder::visitClassDeclarationStatement(
            parsers::BrainfuckLangParser::ClassDeclarationStatementContext *ctx) {
        // declare
        auto symbol = SymbolTable::declare(ctx->classDeclaration(), ctx->classImplementation()).resolvedSymbol;

        std::cout << symbol->firstDeclaration() << "\tclass\t\t" << symbol->qualifiedId() << std::endl;

        // implement
        if (ctx->classImplementation()) {
            Compiler::implementClass(symbol->as<ClassSymbol>(), ctx->classImplementation());
        }

        return symbol;
    }

    antlrcpp::Any brainfuck::compiler::SymbolTableBuilder::visitVariableDeclarationStatement(
            parsers::BrainfuckLangParser::VariableDeclarationStatementContext *ctx) {
        return VariableSymbolDeclarationVisitor(ctx).visit(ctx);
    }

    antlrcpp::Any brainfuck::compiler::SymbolTableBuilder::visitVariableDeclarationList(
            parsers::BrainfuckLangParser::VariableDeclarationListContext *ctx) {
        return VariableSymbolDeclarationVisitor(nullptr).visit(ctx);
    }

    antlrcpp::Any SymbolTableBuilder::visitBlockStatement(parsers::BrainfuckLangParser::BlockStatementContext *ctx) {
        auto block = SymbolTable::declareBlockScope(ctx);
        SymbolTable::push(block);
        visitChildren(ctx);
        SymbolTable::pop();
        return block;
    }

    antlrcpp::Any SymbolTableBuilder::visitAliasDeclarationStatement(
            parsers::BrainfuckLangParser::AliasDeclarationStatementContext *ctx) {
        auto symbol = SymbolTable::getTop()->create(ctx->aliasDeclaration()->IDENTIFIER()->getText(),
                                                   std::unique_ptr<Symbol>(new AliasSymbol));
        symbol->declarationLocations.emplace_back(Compiler::currentFile(), ctx);
        symbol->setCompleted(ctx);
        symbol->as<AliasSymbol>()->type = nullptr;
        std::cout << symbol->firstDeclaration() << "\talias\t\t" << symbol->qualifiedId() << std::dec << std::endl;
        return symbol;
    }

    antlrcpp::Any brainfuck::compiler::VariableSymbolDeclarationVisitor::visitVariableDeclaration(
            parsers::BrainfuckLangParser::VariableDeclarationContext *ctx) {
        // declare classes in the signature
        SymbolTableBuilder().visit(ctx->typeSignature());
        // declare variable on top
        auto symbol = SymbolTable::getTop()->declare(ctx, this->stmt ? VariableQualifiers(this->stmt->variableQualifier()) : VariableQualifiers());
        std::cout << symbol->firstDeclaration() << "\tvariable\t" << symbol->qualifiedId() << "@" << symbol->address << std::endl;
        return symbol;
    }

}
