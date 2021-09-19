//
// Created by Marian Plivelic on 28.04.18.
//

#include "ClassSymbol.h"

bool brainfuck::compiler::ClassSymbol::operator==(const brainfuck::compiler::Symbol &that) const {
    return dynamic_cast<const ClassSymbol *>(&that) && Symbol::operator==(that);
}

std::string brainfuck::compiler::ClassSymbol::qualifiedId() const {
    return (isInterface ? "interface " : "") + Symbol::qualifiedId();
}

brainfuck::compiler::ClassSymbol::ClassSymbol() = default;

brainfuck::compiler::ClassSymbol::~ClassSymbol() = default;
