//
// Created by Marian Plivelic on 28.04.18.
//

#ifndef BRAINFUCKLANG_CLASSSYMBOL_H
#define BRAINFUCKLANG_CLASSSYMBOL_H
#include "brainfuck.h"
#include "Environment.h"
#include "forward.h"

namespace brainfuck::compiler {
    struct ClassSymbol : public Environment {
        ClassSymbol();
        ~ClassSymbol() override;

        /** @brief Interface classes may not contain member variables */
        bool isInterface = false;

        /** @brief List of implemented interfaces */
        std::vector<ClassSymbol*> inherits;

        std::string qualifiedId() const override;

        bool operator==(const Symbol &that) const override;
    };
}


#endif //BRAINFUCKLANG_CLASSSYMBOL_H
