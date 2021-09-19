//
// Created by Marian Plivelic on 24.04.18.
//

#ifndef BRAINFUCKLANG_SYMBOLTABLEBUILDER_H
#define BRAINFUCKLANG_SYMBOLTABLEBUILDER_H

#include "brainfuck.h"
#include "forward.h"
#include "Environment.h"

namespace brainfuck::compiler {
    class VariableSymbolDeclarationVisitor : public parsers::BrainfuckLangBaseVisitor {
        parsers::BrainfuckLangParser::VariableDeclarationStatementContext* stmt;
    public:
        VariableSymbolDeclarationVisitor(parsers::BrainfuckLangParser::VariableDeclarationStatementContext* stmt)
            : stmt(stmt) { }
        antlrcpp::Any
        visitVariableDeclaration(parsers::BrainfuckLangParser::VariableDeclarationContext *ctx) override;
    };

    /**
     * The builder goes through the context and adds all (nested) symbols
     */
    class SymbolTableBuilder : public parsers::BrainfuckLangBaseVisitor  {
    public:
        antlrcpp::Any visitClassDeclarationStatement(
                parsers::BrainfuckLangParser::ClassDeclarationStatementContext *ctx) override;

        antlrcpp::Any
        visitAliasDeclarationStatement(parsers::BrainfuckLangParser::AliasDeclarationStatementContext *ctx) override;

        antlrcpp::Any visitBlockStatement(parsers::BrainfuckLangParser::BlockStatementContext *ctx) override;

        antlrcpp::Any visitFunctionDeclarationStatement(
                parsers::BrainfuckLangParser::FunctionDeclarationStatementContext *ctx) override;

        antlrcpp::Any
        visitClassDeclarationTypeName(parsers::BrainfuckLangParser::ClassDeclarationTypeNameContext *ctx) override;

        antlrcpp::Any
        visitVariableDeclarationList(parsers::BrainfuckLangParser::VariableDeclarationListContext *ctx) override;

        antlrcpp::Any visitVariableDeclarationStatement(
                parsers::BrainfuckLangParser::VariableDeclarationStatementContext *ctx) override;

    };
}


#endif //BRAINFUCKLANG_SYMBOLTABLEBUILDER_H
