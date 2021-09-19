/**
 * Main function for my brainfuck language compiler
 */
#include <memory>
#include "brainfuck.h"
#include "bflang/Compiler.h"

using namespace antlr4;
using namespace brainfuck::parsers;
using namespace brainfuck::util;
using namespace brainfuck::compiler;

int main(int argc, char *argv[]) {
    Compiler::compile("../examples/bl/basic-declarations.bl");
    return 0;
}
