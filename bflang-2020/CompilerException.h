//
// Created by Marian Plivelic on 25.04.18.
//

#ifndef BRAINFUCKLANG_ERRORS_H
#define BRAINFUCKLANG_ERRORS_H

#include <exception>
#include <string>

namespace brainfuck {
    struct Location;
}

namespace brainfuck::exceptions {

    enum class ErrorSeverity {
        NONE, HINT, WARNING, ERROR
    };

    struct CompilerException : std::exception {
        ErrorSeverity severity = ErrorSeverity::ERROR;
        std::string message = "<no message>";

        void emit(const Location &loc);

        const char *what() const noexcept override;
    };

    class InvalidFlagArgumentException : public CompilerException {
        std::string flag;
    public:
        explicit InvalidFlagArgumentException(std::string flag) : flag(std::move(flag)) {
            message = "Invalid flag '" + flag + "' provided";
        }
    };

    /**
     * Emits an error message and exits the program if the severity is ERROR
     * @param e Exception to emit
     * @param loc Error message reference location
     */
    void emit(const CompilerException &e, const Location &loc);

    /**
     * emit error and exit.
     * @param ctx Context for location
     * @param message Message to write
     */
    void emit(antlr4::ParserRuleContext *ctx, std::string message);
}


#endif //BRAINFUCKLANG_ERRORS_H
