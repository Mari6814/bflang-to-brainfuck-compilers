//
// Created by Marian Plivelic on 28.04.18.
//

#include "Type.h"
#include <memory>
#include "CompilerException.h"
#include "FunctionSymbol.h"
#include "Symbol.h"
#include "ClassSymbol.h"
#include "Compiler.h"

brainfuck::compiler::TypeQualifiers &brainfuck::compiler::TypeQualifiers::operator+=(const std::string &flag) {
    if (flag == "const") hasConst = true;
    else if (flag == "constexpr") hasConst = hasConstexpr = true;
    else throw exceptions::InvalidFlagArgumentException(flag);
    return *this;
}

bool brainfuck::compiler::Type::operator==(const brainfuck::compiler::Type &that) const {
    return qualifiers == that.qualifiers;
}

std::unique_ptr<brainfuck::compiler::Type>
brainfuck::compiler::Type::create(brainfuck::parsers::BrainfuckLangParser::TypeSignatureContext *ctx) {
    if (ctx->functionPointerTypeSignature()) {
        throw std::exception();
    } else {
        std::unique_ptr<Type> type;
        if (ctx->typeNameTypeSignature()->typeName()->isVoid)
            type = std::make_unique<VoidType>(ctx->typeNameTypeSignature());
        else if (ctx->typeNameTypeSignature()->typeName()->isAuto)
            type = std::make_unique<AutoType>(ctx->typeNameTypeSignature());
        else if (ctx->typeNameTypeSignature()->typeName()->nativeTypeName())
            type = std::make_unique<NativeType>(ctx->typeNameTypeSignature());
        else if (ctx->typeNameTypeSignature()->typeName()->qualifiedIdTypeName())
            type = std::make_unique<UserType>(ctx->typeNameTypeSignature());
        else if (ctx->typeNameTypeSignature()->typeName()->classDeclarationTypeName())
            type = std::make_unique<UserType>(ctx->typeNameTypeSignature());
        else throw std::exception();

        auto pointers = ctx->typeNameTypeSignature()->pointer();
        if (!pointers.empty()) {
            for (auto pCtx : pointers)
                type = std::make_unique<ArrayType>(pCtx->typeQualifier(), pCtx->expr() != nullptr, 0, std::move(type));
        }
        return type;
    }
}

brainfuck::compiler::Type::Type(
        const std::vector<brainfuck::parsers::BrainfuckLangParser::TypeQualifierContext *> &qualifiers) {
    for (auto q : qualifiers)
        this->qualifiers += q->getText();
}

std::string brainfuck::compiler::Type::str() const {
    std::stringstream ss;
    if (qualifiers.any())
        ss << " " << qualifiers;
    return ss.str();
}

size_t brainfuck::compiler::VoidType::getSize(antlr4::ParserRuleContext *location) {
    exceptions::emit(location, "Void has no size");
    // never reached
    exit(-1);
}

bool brainfuck::compiler::VoidType::operator==(const brainfuck::compiler::Type &that) const {
    return nullptr != dynamic_cast<const VoidType*>(&that) && Type::operator==(that);
}

brainfuck::compiler::VoidType::VoidType(brainfuck::parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx)
        : Type(ctx->typeQualifier()) {

}

std::string brainfuck::compiler::VoidType::str() const {
    std::string base = Type::str();
    if (base.empty())
        return "void";
    return "void " + base;
}

size_t brainfuck::compiler::AutoType::getSize(antlr4::ParserRuleContext *location) {
    if (!type)
        exceptions::emit(location, "Size of auto type could not be deduced");
    return type->getSize(location);
}

std::string brainfuck::compiler::AutoType::str() const {
    std::string base = type ? type->str() : "auto";
    std::string that = Type::str();
    if (that.empty())
        return base;
    return base + " " + that;
}

bool brainfuck::compiler::AutoType::operator==(const brainfuck::compiler::Type &that) const {
    auto *t = dynamic_cast<const AutoType*>(&that);
    if (t == nullptr)
        return false;
    if ((type.get() == nullptr) != (t->type.get() == nullptr))
        return false;
    if (!t->type->operator==(*type))
        return false;
    return Type::operator==(that);
}

brainfuck::compiler::AutoType::AutoType(brainfuck::parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx)
    : Type(ctx->typeQualifier()), type(nullptr) {
}

size_t brainfuck::compiler::UserType::getSize(antlr4::ParserRuleContext *location) {
    if (!symbol->isComplete)
        exceptions::emit(location, "Can't find size of incomplete type " + symbol->qualifiedId());
    return symbol->offset;
}

std::string brainfuck::compiler::UserType::str() const {
    return symbol->qualifiedId() + Type::str();
}

bool brainfuck::compiler::UserType::operator==(const brainfuck::compiler::Type &that) const {
    return nullptr != dynamic_cast<const UserType*>(&that) && dynamic_cast<const UserType&>(that).symbol == symbol
           && Type::operator==(that);
}

brainfuck::compiler::UserType::UserType(brainfuck::parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx)
    : Type(ctx->typeQualifier()) {
    assert(ctx->typeName());
    assert(ctx->typeName()->classDeclarationTypeName()
           || ctx->typeName()->qualifiedIdTypeName());
    if (ctx->typeName()->classDeclarationTypeName()) {
        auto qid = ctx->typeName()->classDeclarationTypeName()->classDeclaration()->qualifiedId();
        if (!qid) {
            // anonymous
            this->symbol = SymbolTable::getTop()->resolve(ctx->typeName()->classDeclarationTypeName()->classDeclaration());
        } else {
            // defined somewhere
            auto path = SymbolTable::resolve(qid);
            if (path.matching.size() > 1)
                exceptions::emit(ctx, "Non unique symbol id " + qid->getText());
            if (!path.resolvedSymbol)
                exceptions::emit(ctx, "Undefined symbol " + qid->getText());
            if (!path.allOf<ClassSymbol>())
                exceptions::emit(ctx, "Class symbol expected");
            this->symbol = path.resolvedSymbol->as<ClassSymbol>();
        }
    } else if (ctx->typeName()->qualifiedIdTypeName()) {
        auto qid = ctx->typeName()->qualifiedIdTypeName()->qualifiedId();
        auto path = SymbolTable::resolve(qid);
        if (!path.resolvedSymbol)
            exceptions::emit(ctx, "Undefined symbol " + qid->getText());
        if (path.resolvedSymbol->is<FunctionSymbol>() && path.resolvedSymbol->as<FunctionSymbol>()->isSpecialMember()) {
            // resolve special member type
            assert(path.resolvedSymbol->parent->is<ClassSymbol>());
            this->symbol = path.resolvedSymbol->parent->as<ClassSymbol>();
        } else if (!path.allOf<ClassSymbol>())
            exceptions::emit(ctx, "Class symbol expected");
        else
            this->symbol = path.resolvedSymbol->as<ClassSymbol>();
    }
}

size_t brainfuck::compiler::ArrayType::getSize(antlr4::ParserRuleContext *location) {
    if (fixed)
        return length * type->getSize(location);
    return Compiler::settings.pointerSize;
}

std::string brainfuck::compiler::ArrayType::str() const {
    return type->str() + "[" + (fixed ? std::to_string(length) : "") + "]" + Type::str();
}

std::string brainfuck::compiler::NativeType::str() const {
    std::string base = Type::str();
    switch (sign)
    {
        case DEFAULT: return "cell" + Type::str();
        case SIGNED: return "signed cell" + Type::str();
        case UNSIGNED: return "unsigned cell" + Type::str();
        default:
            return "cell" + Type::str();
    }
}

brainfuck::compiler::NativeType::NativeType(brainfuck::parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx)
        : Type(ctx->typeQualifier()) {
    assert(ctx);
    assert(ctx->typeName());
    assert(ctx->typeName()->nativeTypeName());
    if (ctx->typeName()->nativeTypeName()->typeSign()) {
        auto sign = ctx->typeName()->nativeTypeName()->typeSign()->getText();
        if (sign == "signed")
            this->sign = Sign::SIGNED;
        else if (sign == "unsigned")
            this->sign = Sign::UNSIGNED;
        else
            this->sign = Sign::DEFAULT;
    }
}

bool brainfuck::compiler::NativeType::operator==(const brainfuck::compiler::Type &that) const {
    return dynamic_cast<const NativeType*>(&that) != nullptr
           && Type::operator==(that);
}
