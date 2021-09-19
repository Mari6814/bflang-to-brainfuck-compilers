//
// Created by Marian Plivelic on 25.04.18.
//

#include <iostream>
#include <string>
#include "Location.h"
#include "CompilerException.h"
#include "bflang/Compiler.h"

void brainfuck::exceptions::emit(const exceptions::CompilerException &e, const brainfuck::Location &loc) {
    std::stringstream ss;
    ss << loc;
    switch (e.severity) {
        case brainfuck::exceptions::ErrorSeverity::HINT:
            ss << " \033[37;mhint: ";
            break;
        case brainfuck::exceptions::ErrorSeverity::WARNING:
            ss << " \033[32;mwarning: ";
            break;
        case brainfuck::exceptions::ErrorSeverity::ERROR:
            ss << " \033[31;merror: ";
            break;
        default:
            ss << "critical: ";
    }
    ss << e.message << "\033[0m" << std::endl;
    std::cout.flush();
    std::cerr.flush();
    std::cerr << ss.str();
    if (e.severity == brainfuck::exceptions::ErrorSeverity::ERROR)
        exit(-1);
}

void brainfuck::exceptions::emit(antlr4::ParserRuleContext *ctx, std::string message) {
    CompilerException err;
    err.severity = ErrorSeverity::ERROR;
    err.message = std::move(message);
    emit(err, Location(compiler::Compiler::currentFile(), ctx));
}

const char *brainfuck::exceptions::CompilerException::what() const noexcept {
    return message.c_str();
}

void brainfuck::exceptions::CompilerException::emit(const brainfuck::Location &loc) {
    brainfuck::exceptions::emit(*this, loc);
}
