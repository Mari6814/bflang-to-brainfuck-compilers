//
// Created by Marian Plivelic on 22.03.18.
//

#ifndef BRAINFUCKLANG_ASSEMBLY_H
#define BRAINFUCKLANG_ASSEMBLY_H

#include "brainfuck.h"

namespace brainfuck::assembly {
    /**
     * Alias for the antlr4 assembly parser
     */
    using Parser = brainfuck::parsers::BrainfuckLangAssemblyParser;
}

#endif //BRAINFUCKLANG_ASSEMBLY_H
