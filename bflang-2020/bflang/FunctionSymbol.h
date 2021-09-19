//
// Created by Marian Plivelic on 28.04.18.
//

#ifndef BRAINFUCKLANG_FUNCTIONSYMBOL_H
#define BRAINFUCKLANG_FUNCTIONSYMBOL_H

#include "brainfuck.h"
#include "Symbol.h"
#include "Environment.h"

namespace brainfuck::compiler {

    /**
     * Flags for function call semantic, storage specification or compiler hint
     */
    struct FunctionSpecifiers {
        bool hasStatic = false;
        bool hasAbstract = false;
        bool hasOverride = false;
        bool hasGet = false;
        bool hasSet = false;
        bool hasInline = false;

        FunctionSpecifiers() = default;
        FunctionSpecifiers(const std::vector<parsers::BrainfuckLangParser::FunctionSpecifierContext*> &specifiers);

        FunctionSpecifiers &operator+=(const std::string &flag);

        friend std::ostream &operator<<(std::ostream &os, const FunctionSpecifiers &that);

        bool operator==(const FunctionSpecifiers &that) const {
            return hasStatic == that.hasStatic
                   && hasAbstract == that.hasAbstract
                   && hasOverride == that.hasOverride
                   && hasGet == that.hasGet
                   && hasSet == that.hasSet
                   && hasInline == that.hasInline;
        }

        bool operator!=(const FunctionSpecifiers &that) const { return !(*this == that); }
    };

    struct FunctionQualifiers {
        bool hasConst = false;
        bool hasConstexpr = false;

        /** @brief Adds a qualifier */
        FunctionQualifiers &operator+=(const std::string &flag);

        /** @return True if any qualifier has been added */
        bool any() const {
            return hasConst || hasConstexpr;
        }

        bool operator==(const FunctionQualifiers &that) const {
            return hasConst == that.hasConst && hasConstexpr == that.hasConstexpr;
        }

        bool operator!=(const FunctionQualifiers &that) const { return !(*this == that); }

        friend std::ostream &operator<<(std::ostream &os, const FunctionQualifiers &that);
    };

    /** @brief Information about only the data needed to call and return from a function */
    struct FunctionSignature {
        /** @brief Types for the parameters */
        std::vector<std::unique_ptr<Type>> parameterTypes;

        /** @brief Types for the return values */
        std::vector<std::unique_ptr<Type>> returnTypes;

        /** @brief Function signature qualifiers like 'const' */
        FunctionQualifiers qualifiers;

        /** @return True if return types are the same */
        bool equalReturnTypes(FunctionSignature *that) const;

        /** @return String of only the parameters and qualifiers */
        std::string parametersToString() const;

        explicit FunctionSignature(parsers::BrainfuckLangParser::FunctionSignatureContext *ctx);

        friend std::ostream &operator<<(std::ostream &os, const FunctionSignature &that);

        bool operator==(const FunctionSignature &that) const;

        bool operator!=(const FunctionSignature &that) const { return !(*this == that); }
    };

    struct FunctionSymbol : public Environment {
        /** @brief Function specifiers that are not part of the signature */
        FunctionSpecifiers specifiers;

        /** @brief Flag set if declared inside as a member function inside a class or interface */
        bool isMember = false;

        /** @return True if is special member constructor */
        bool isConstructor = false;

        /** @return True if is special member destructor */
        bool isDestructor = false;

        /** @return True if either ctor or dtor */
        bool isSpecialMember() const { return isConstructor != isDestructor; }

        /** @copybrief */
        std::string qualifiedId() const override;

        /** @brief Information needed to call a function */
        std::unique_ptr<FunctionSignature> signature;

        bool operator==(const Symbol &that) const override;

        bool operator!=(const Symbol &that) const { return !(*this == that); }

        friend std::ostream &operator<<(std::ostream &os, const FunctionSymbol &fun);
    };
}


#endif //BRAINFUCKLANG_FUNCTIONSYMBOL_H
