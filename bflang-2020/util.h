//
// Created by Marian Plivelic on 21.02.18.
//

#ifndef BF2_UTIL_H
#define BF2_UTIL_H

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>
#include <numeric>
#include <functional>
#include <antlr4-runtime.h>
#include <parsers/BrainfuckLangParser.h>

namespace brainfuck {
    struct Location;
}

namespace brainfuck::util {
    namespace parser {
        /**
         * @brief Manages the structures required for parsing with antlr4
         * @tparam Lexer Lexer to use
         * @tparam Parser Parser to use
         */
        template<typename Lexer, typename Parser>
        struct ParserUtil {
            /**
             * @brief Creates a parser for the specified sourcepath
             * @param sourcepath Path to sourcefile
             * @return Parsetree
             * @throw std::exception if a file is already open
             */
            brainfuck::parsers::BrainfuckLangParser::MainContext *parse(const char *sourcepath) {
                std::fstream file(sourcepath);
                if (!file.is_open())
                    throw std::runtime_error("File '" + std::string(sourcepath) + "' not found!");
                is = std::make_unique<antlr4::ANTLRInputStream>(file);
                lexer = std::make_unique<Lexer>(is.get());
                tokens = std::make_unique<antlr4::CommonTokenStream>(lexer.get());
                tokens->fill();
                parser = std::make_unique<Parser>(tokens.get());
                return parser.get()->main();
            }
        private:
            std::unique_ptr<antlr4::ANTLRInputStream> is;
            std::unique_ptr<Lexer> lexer;
            std::unique_ptr<antlr4::CommonTokenStream> tokens;
            std::unique_ptr<Parser> parser;
        };
    }
}

#endif //BF2_UTIL_H
