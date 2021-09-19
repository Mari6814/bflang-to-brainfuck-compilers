//
// Created by Marian Plivelic on 15.04.18.
//

#ifndef BRAINFUCKLANG_LOCATION_H
#define BRAINFUCKLANG_LOCATION_H
#include <string>
#include <vector>
#include <iostream>
#include "brainfuck.h"
namespace brainfuck {

    /** @brief Location of a single token */
    struct TokenLocation {
        /** @brief Line in the file */
        size_t line;

        /** @brief Column in the line */
        size_t column;

        /** @brief 0 constructor */
        TokenLocation() : line(0), column(0) { }

        /** @brief Construct with token given */
        explicit TokenLocation(antlr4::Token *tok)
                : line(tok->getLine()), column(tok->getCharPositionInLine() + 1) { }

        bool operator==(const TokenLocation &that) const { return line == that.line && column == that.column; }

        bool operator!=(const TokenLocation &that) const { return !(*this == that); }

        friend std::ostream &operator<<(std::ostream &os, const TokenLocation &that) {
            return os << that.line << ":" << that.column;
        }
    };

    /** @brief Basic location information */
    struct Location {
        /** @brief Path to file */
        std::string file;
        /** @brief Location of the token at the beginning */
        TokenLocation begin;
        /** @brief Location of the token at the end */
        TokenLocation end;

        /** @brief full constructor */
        Location(std::string file, TokenLocation begin, TokenLocation end)
                : file(std::move(file)), begin(begin), end(end) { }

        /** @brief Copies location from context start and stop tokens */
        Location(std::string file, antlr4::ParserRuleContext *ctx)
            : Location(std::move(file), TokenLocation(ctx->start), TokenLocation(ctx->stop)) { }

        /** @brief invalid location constructor */
        Location() : Location(std::string(), TokenLocation(), TokenLocation()) { }

        /** @return Valid if file name is specified */
        bool isValid() const { return !file.empty(); }

        /** @return isValid() */
        explicit operator bool() const { return isValid(); }

        /** @return True if either both are invalid or all information is equal */
        bool operator==(const Location &that) const {
            return begin == that.begin && end == that.end && file == that.file;
        }

        /** @return negation of operator==() */
        bool operator!=(const Location &that) const { return !(*this == that); }

        /** @brief stream operator */
        friend std::ostream &operator<<(std::ostream &os, const Location &that) {
            if (!that.isValid())
                return os << "<unknown>:?:?";
            return os << that.file << ":" << that.begin;
        }

        operator std::string() const {
            std::stringstream ss;
            ss << *this;
            return ss.str();
        }
    };
}


#endif //BRAINFUCKLANG_LOCATION_H
