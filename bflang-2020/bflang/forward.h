//
// Created by Marian Plivelic on 15.04.18.
//

#ifndef BRAINFUCKLANG_FORWARD_H
#define BRAINFUCKLANG_FORWARD_H
#include <string>

namespace brainfuck::compiler {
    /** @brief Alias for strings that reference symbol ids */
    using id_t = std::string;

    class Compiler;
    class Environment;
    class SymbolDeclarationStack;

    struct Type;
    struct VoidType;
    struct AutoType;
    struct NativeType;
    struct UserType;
    struct FunctionPointerType;
    struct ArrayType;
    struct FunctionSignature;
    struct VariableQualifiers;
    struct TypeQualifiers;
    struct FunctionSpecifiers;
    struct FunctionQualifiers;
    struct Symbol;
    struct FunctionSymbol;
    struct VariableSymbol;
    struct AliasSymbol;
    struct NamespaceSymbol;
    struct ClassSymbol;
    struct EnumSymbol;
    struct TemplateSymbol;
    struct TemplateParameterSymbol;
    struct TemplateParameterTypeSymbol;
    struct TemplateParameterExpressionSymbol;
}

#endif //BRAINFUCKLANG_FORWARD_H
