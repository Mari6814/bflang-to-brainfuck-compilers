//
// Created by Marian Plivelic on 28.04.18.
//

#include "FunctionSymbol.h"
#include "CompilerException.h"
#include "Type.h"

brainfuck::compiler::FunctionSpecifiers &brainfuck::compiler::FunctionSpecifiers::operator+=(const std::string &flag) {
    if (flag == "static") hasStatic = true;
    else if (flag == "abstract") hasAbstract = true;
    else if (flag == "override") hasOverride = true;
    else if (flag == "get") hasGet = true;
    else if (flag == "set") hasSet = true;
    else if (flag == "inline") hasInline = true;
    else throw exceptions::InvalidFlagArgumentException(flag);
    return *this;
}

std::ostream &brainfuck::compiler::operator<<(std::ostream &os, const brainfuck::compiler::FunctionSpecifiers &that) {
    std::vector<std::string> str;
    if (that.hasInline) str.emplace_back("inline");
    if (that.hasStatic) str.emplace_back("static");
    if (that.hasAbstract) str.emplace_back("abstract");
    if (that.hasOverride) str.emplace_back("override");
    if (that.hasGet) str.emplace_back("get");
    if (that.hasSet) str.emplace_back("set");
    std::copy(str.begin(), str.end(), std::ostream_iterator<std::string>(os, " "));
    return os;
}

brainfuck::compiler::FunctionSpecifiers::FunctionSpecifiers(
        const std::vector<brainfuck::parsers::BrainfuckLangParser::FunctionSpecifierContext *> &specifiers) {
    for (auto ctx : specifiers)
        *this += ctx->getText();
}

brainfuck::compiler::FunctionQualifiers &brainfuck::compiler::FunctionQualifiers::operator+=(const std::string &flag) {
    if (flag == "const") hasConst = true;
    else if (flag == "constexpr") hasConst = hasConstexpr = true;
    else throw exceptions::InvalidFlagArgumentException(flag);
    return *this;
}

std::ostream &brainfuck::compiler::operator<<(std::ostream &os, const brainfuck::compiler::FunctionQualifiers &that) {
    if (that.hasConstexpr) return os << "constexpr";
    else if (that.hasConst) return os << "const";
    return os;
}

bool brainfuck::compiler::FunctionSymbol::operator==(const brainfuck::compiler::Symbol &that) const {
    if (!Symbol::operator==(that))
        return false;
    auto thatf = dynamic_cast<const FunctionSymbol*>(&that);
    // specifiers do not matter for function symbol equality
    // specifiers == thatf->specifiers
    return thatf != nullptr && *signature == *thatf->signature;
}

std::ostream &brainfuck::compiler::operator<<(std::ostream &os, const brainfuck::compiler::FunctionSymbol &fun) {
    return os << fun.specifiers << fun.qualifiedId() << *fun.signature;
}

std::string brainfuck::compiler::FunctionSymbol::qualifiedId() const {
    std::stringstream ss;
    ss << specifiers << Symbol::qualifiedId() << signature->parametersToString();
    return ss.str();
}

bool brainfuck::compiler::FunctionSignature::operator==(const brainfuck::compiler::FunctionSignature &that) const {
    if (parameterTypes.size() != that.parameterTypes.size())
        return false;
    for (int i = 0; i < parameterTypes.size(); ++i)
        if (*parameterTypes[i] != *that.parameterTypes[i])
            return false;
    return qualifiers == that.qualifiers;
}

brainfuck::compiler::FunctionSignature::FunctionSignature(
        brainfuck::parsers::BrainfuckLangParser::FunctionSignatureContext *ctx) {
    for (auto q : ctx->functionQualifier())
        this->qualifiers += q->getText();

    if (ctx->functionParameters() && ctx->functionParameters()->variableDeclarationList()) {
        for (auto decl : ctx->functionParameters()->variableDeclarationList()->variableDeclaration()) {
            this->parameterTypes.push_back(std::move(Type::create(decl->typeSignature())));
            this->parameterTypes.back()->qualifiers.hasConstexpr = this->qualifiers.hasConstexpr;
        }
    }

    if (ctx->functionReturnType() && ctx->functionReturnType()->variableDeclarationList()) {
        for (auto decl : ctx->functionReturnType()->variableDeclarationList()->variableDeclaration()) {
            this->returnTypes.push_back(std::move(Type::create(decl->typeSignature())));
            this->returnTypes.back()->qualifiers.hasConstexpr = this->qualifiers.hasConstexpr;
        }
    }
}

std::ostream &brainfuck::compiler::operator<<(std::ostream &os, const brainfuck::compiler::FunctionSignature &that) {
    os << "(";
    for (const auto &pt : that.parameterTypes)
        os << (pt == that.parameterTypes.front() ? "" : ", " ) << pt->str();
    os << ")" << that.qualifiers;
    for (const auto &pt : that.returnTypes)
        os << (pt == that.returnTypes.front() ? " -> " : ", " ) << pt->str();
    return os;
}

bool brainfuck::compiler::FunctionSignature::equalReturnTypes(brainfuck::compiler::FunctionSignature *that) const {
    if (returnTypes.size() != that->returnTypes.size())
        return false;
    for (int i = 0; i < returnTypes.size(); ++i)
        if (*returnTypes[i] != *that->returnTypes[i])
            return false;
    return true;
}

std::string brainfuck::compiler::FunctionSignature::parametersToString() const {
    std::stringstream ss;
    ss << "(";
    for (const auto &param : parameterTypes) {
        ss << (parameterTypes.front().get() == param.get() ? "" : ", ")
           << param->str();
    }
    ss << ")";
    if (this->qualifiers.any())
        ss << " " << this->qualifiers;
    return ss.str();
}
