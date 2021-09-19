//
// Created by Marian Plivelic on 09.04.18.
//

#include "Symbol.h"
#include "Environment.h"
#include "CompilerException.h"
#include "Compiler.h"

std::string brainfuck::compiler::Symbol::qualifiedId() const {
    if (parent) {
        if (parent->parent)
            return parent->qualifiedId() + "::" + id;
        return id;
    }
    return "";
}

void brainfuck::compiler::Symbol::setCompleted(antlr4::ParserRuleContext *implementationLocationContext) {
    // provided location
    auto loc = Location(Compiler::currentFile(), implementationLocationContext);
    // prevent double completion
    if (isComplete)
        exceptions::emit(implementationLocationContext, "Symbol has been completed before at " + std::string(loc));
    // mark completed
    isComplete = true;
    implementationLocation = std::move(loc);
}
