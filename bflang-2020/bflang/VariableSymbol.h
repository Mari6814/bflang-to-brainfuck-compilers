//
// Created by Marian Plivelic on 28.04.18.
//

#ifndef BRAINFUCKLANG_VARIABLESYMBOL_H
#define BRAINFUCKLANG_VARIABLESYMBOL_H

#include "brainfuck.h"
#include "Symbol.h"
#include "Type.h"

namespace brainfuck::compiler {

    struct VariableQualifiers {
        bool hasStatic = false;
        bool hasConst = false;
        bool hasConstexpr = false;
        bool hasVar = false;

        VariableQualifiers() = default;
        explicit VariableQualifiers(const std::vector<parsers::BrainfuckLangParser::VariableQualifierContext*> &qualifiers);

        /** @return Returns true if any flag is set */
        bool any() const {
            return hasStatic || hasConstexpr || hasConstexpr;
        }

        /** @brief Adds a qualifier */
        VariableQualifiers &operator+=(const std::string &flag);

        /** @brief True if equal */
        bool operator==(const VariableQualifiers &that) const;

        friend std::ostream &operator<<(std::ostream &os, const VariableQualifiers &that);
    };

    struct VariableSymbol : Symbol {
        /** @brief Type */
        std::unique_ptr<Type> type;

        /** @brief Qualifiers */
        VariableQualifiers qualifiers;

        std::string qualifiedId() const override;

        /** @brief address on stack */
        size_t address = 0;

        bool operator==(const Symbol &that) const override;
    };
}


#endif //BRAINFUCKLANG_VARIABLESYMBOL_H
