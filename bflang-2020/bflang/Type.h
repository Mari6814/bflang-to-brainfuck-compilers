//
// Created by Marian Plivelic on 28.04.18.
//

#ifndef BRAINFUCKLANG_TYPE_H
#define BRAINFUCKLANG_TYPE_H

#include "brainfuck.h"
#include "forward.h"
#include "Compiler.h"

namespace brainfuck::compiler {

    /**
     * Qualifiers for types and pointers
     */
    struct TypeQualifiers {
        bool hasConst = false;
        bool hasConstexpr = false;

        /** @return True if at least once qualifiers was added */
        bool any() const {
            return hasConst || hasConstexpr;
        }

        /** @return Returns true if equal */
        bool operator==(const TypeQualifiers &that) const {
            return hasConst == that.hasConst && hasConstexpr == that.hasConstexpr;
        }

        /** @brief Adds a qualifier */
        TypeQualifiers &operator+=(const std::string &flag);

        friend std::ostream &operator<<(std::ostream &os, const TypeQualifiers &that) {
            if (that.hasConstexpr)
                return os << "constexpr";
            else if (that.hasConst)
                return os << "const";
            return os;
        }
    };

    /**
     * Type signature adds Array
     */
    struct Type {
        /** @brief Qualifiers like const etc */
        TypeQualifiers qualifiers;

        Type() = default;

        /** @brief builds type qualifiers */
        explicit Type(const std::vector<parsers::BrainfuckLangParser::TypeQualifierContext*> &qualifiers);

        /** @brief Size on stack */
        virtual size_t getSize(antlr4::ParserRuleContext *location) = 0;

        /** @brief Gets a string representation of the type */
        virtual std::string str() const;

        virtual bool operator==(const Type &that) const;

        bool operator!=(const Type &that) const { return !(*this == that); }

        /** @return Creates type */
        static std::unique_ptr<Type> create(parsers::BrainfuckLangParser::TypeSignatureContext *ctx);
    };

    struct VoidType : Type {
        size_t getSize(antlr4::ParserRuleContext *location) override;

        std::string str() const override;

        explicit VoidType(parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx);

        bool operator==(const Type &that) const override;
    };

    struct AutoType : Type {
        std::unique_ptr<Type> type = nullptr;

        size_t getSize(antlr4::ParserRuleContext *location) override;

        std::string str() const override;

        explicit AutoType(parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx);

        bool operator==(const Type &that) const override;
    };

    struct NativeType : Type {
        /** @brief Sign of the cell */
        enum Sign {
            DEFAULT, SIGNED, UNSIGNED
        } sign = DEFAULT;

        size_t getSize(antlr4::ParserRuleContext *location) override  { return 1; }

        std::string str() const override;

        explicit NativeType(parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx);

        bool operator==(const Type &that) const override;
    };

    struct UserType : Type {
        ClassSymbol *symbol = nullptr;

        size_t getSize(antlr4::ParserRuleContext *location) override;

        std::string str() const override;

        explicit UserType(parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx);

        bool operator==(const Type &that) const override;
    };

    struct FunctionPointerType : Type {
        std::unique_ptr<FunctionSignature> signature;

        size_t getSize(antlr4::ParserRuleContext *location) override { return Compiler::settings.functionPointerSize; }

        explicit FunctionPointerType(parsers::BrainfuckLangParser::TypeNameTypeSignatureContext *ctx);

        bool operator==(const Type &that) const override;
    };

    struct ArrayType : Type {
        bool fixed = false;
        size_t length = 0;
        std::unique_ptr<Type> type;

        size_t getSize(antlr4::ParserRuleContext *location) override;

        std::string str() const override;

        ArrayType(const std::vector<parsers::BrainfuckLangParser::TypeQualifierContext*> &qualifiers, bool fixed, size_t length, std::unique_ptr<Type> type)
                : Type(qualifiers), fixed(fixed), length(length), type(std::move(type)) { }

        bool operator==(const Type &that) const override {
            return nullptr != dynamic_cast<const ArrayType*>(&that)
                   && dynamic_cast<const ArrayType&>(that).fixed == fixed
                   && dynamic_cast<const ArrayType&>(that).length == length
                   && *dynamic_cast<const ArrayType&>(that).type == *type
                   && Type::operator==(that);
        }

    };
}


#endif //BRAINFUCKLANG_TYPE_H
