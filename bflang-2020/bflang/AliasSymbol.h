//
// Created by Marian Plivelic on 11.05.18.
//

#ifndef BRAINFUCKLANG_ALIASSYMBOL_H
#define BRAINFUCKLANG_ALIASSYMBOL_H
#include "brainfuck.h"
#include "Symbol.h"

namespace brainfuck::compiler {
    class AliasSymbol : public Symbol {
    public:
        std::unique_ptr<Type> type;
    };
}


#endif //BRAINFUCKLANG_ALIASSYMBOL_H
