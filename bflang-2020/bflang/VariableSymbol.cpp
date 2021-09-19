//
// Created by Marian Plivelic on 28.04.18.
//

#include "VariableSymbol.h"
#include "CompilerException.h"

brainfuck::compiler::VariableQualifiers &brainfuck::compiler::VariableQualifiers::operator+=(const std::string &flag) {
    if (flag == "static") hasStatic = true;
    else if (flag == "const") hasConst = true;
    else if (flag == "constexpr") hasConst = hasConstexpr = true;
    else if (flag == "var") hasVar = true;
    else throw exceptions::InvalidFlagArgumentException(flag);
    return *this;
}

bool brainfuck::compiler::VariableQualifiers::operator==(const brainfuck::compiler::VariableQualifiers &that) const {
    return hasStatic == that.hasStatic
           && hasConst == that.hasConst
           && hasConstexpr == that.hasConstexpr;
}

brainfuck::compiler::VariableQualifiers::VariableQualifiers(
        const std::vector<brainfuck::parsers::BrainfuckLangParser::VariableQualifierContext *> &qualifiers) {
    for (auto q : qualifiers)
        *this += q->getText();
}

std::ostream &brainfuck::compiler::operator<<(std::ostream &os, const brainfuck::compiler::VariableQualifiers &that) {
    std::vector<std::string> v;
    if (that.hasConstexpr) v.emplace_back("constexpr");
    else if (that.hasConst) v.emplace_back("const");
    if (that.hasStatic) v.emplace_back("static");
    std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>(os, " "));
    return os;
}

bool brainfuck::compiler::VariableSymbol::operator==(const brainfuck::compiler::Symbol &that) const {
    if (!Symbol::operator==(that))
        return false;
    auto thatv = dynamic_cast<const VariableSymbol*>(&that);
    return thatv && qualifiers == thatv->qualifiers && type == thatv->type;
}

std::string brainfuck::compiler::VariableSymbol::qualifiedId() const {
    std::stringstream ss;
    ss << Symbol::qualifiedId();
    if (qualifiers.any())
        ss << " " << qualifiers;
    return ss.str();
}
