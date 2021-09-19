//
// Created by Marian Plivelic on 09.04.18.
//

#ifndef BRAINFUCKLANG_SYMBOLTABLE_H
#define BRAINFUCKLANG_SYMBOLTABLE_H

#include "brainfuck.h"
#include "forward.h"
#include "CompilerException.h"
#include "Symbol.h"

namespace brainfuck::compiler {
    /**
     * Stores all symbols traversed during resolution step and its results
     */
    struct ResolutionPath {
        /** @brief Traversed symbol tables */
        std::vector<Environment*> path;
        /** @brief List of all matches if not unique */
        std::vector<Symbol*> matching;
        /** @brief Resolved symbol if unique */
        Symbol *resolvedSymbol;

        /** @return Function matching the given signature or null */
        FunctionSymbol *findWithSignature(FunctionSignature *signature);

        /** @return True if all matching symbols are from the same specified symbol type */
        template<typename SymbolType>
        bool allOf() const {
            for (auto s : matching)
                if (!s->is<SymbolType>())
                    return false;
            return true;
        }
    };

    /**
     * Manages symbol information and indexing
     */
    class Environment : public Symbol {
        /** @brief Child symbols */
        std::multimap<id_t, std::unique_ptr<Symbol>> children;

        /** @brief Map of unique void* to anonymous symbols */
        std::map<void*, std::unique_ptr<Symbol>> anonymousSymbols;
    public:
        Environment();
        ~Environment() override;
        Environment(const Environment&) = delete;
        Environment &operator=(const Environment&) = delete;

        /** @brief Sequencial offset counter */
        size_t offset = 0;

        /**
         * @param ctx Qualified id
         * @return Symbol or null
         */
        ResolutionPath resolve(parsers::BrainfuckLangParser::QualifiedIdContext *ctx);

        /**
         * @param ctx Unqualified id context
         * @param reachable Reachable scopes
         * @return Matching symbols
         */
        ResolutionPath resolve(parsers::BrainfuckLangParser::UnqualifiedIdContext *ctx);

        /**
         * @param id Id to resolve
         * @return List of matches or empty vector
         */
        ResolutionPath resolve(id_t id);

        /**
         * @brief Resolves an anonymous class declaration
         * @param anonymousClassDeclaration Declaration context of an anonymous class
         * @return Resolved symbol or nullptr
         */
        ClassSymbol *resolve(parsers::BrainfuckLangParser::ClassDeclarationContext *anonymousClassDeclaration);

        /**
         * Creates an empty symbol from an unqualified id inside the current scope
         * @param ctx Id context
         * @param symbol Symbol to add
         * @param anonymous If symbol should be marked and stored as anonymous
         * @return Create symbol table entry
         */
        Symbol *create(parsers::BrainfuckLangParser::UnqualifiedIdContext *ctx, std::unique_ptr<Symbol> symbol, void *anonymous = nullptr);

        /**
         * Creates a symbol with a given id inside the current scope
         * @param id Id of the new symbol entry
         * @param symbol Symbol to add
         * @param anonymous Flag for marking and storing as an anonymous symbol
         * @return Created symbol table entry
         */
        Symbol *create(id_t id, std::unique_ptr<Symbol> symbol, void *anonymous = nullptr);

        /**
         * Declares a new unique variable symbol inside the current scope.
         * @param ctx Variable declaration context
         * @return Variable symbol
         */
        virtual VariableSymbol *declare(parsers::BrainfuckLangParser::VariableDeclarationContext *ctx, const VariableQualifiers &qualifiers);

        /**
         * Creates new symbol according to the declaration or returns previously declared symbol
         * @param ctx declaration
         * @param impl optional implementation
         * @return Symbol
         */
        virtual ClassSymbol *declare(parsers::BrainfuckLangParser::ClassDeclarationContext *ctx);

        /**
         * Creates new symbol according to the declaration or returns previously declared symbol
         * @param ctx declaration
         * @param impl optional implementation
         * @return Symbol
         */
        virtual FunctionSymbol *declare(parsers::BrainfuckLangParser::FunctionDeclarationContext *ctx);
    };

    class SymbolTable {
        /** @brief Reference frame on the symbol table stack */
        struct Frame {
            /** @brief List of extra reachable scopes for resolution */
            std::vector<Environment*> weakScopes;
            /** @brief The target symbol for new declarations */
            Environment *strongScope;
            explicit Frame(Environment *scope) : strongScope(scope) { }
        };

        /** @brief List of currently available frames */
        static std::vector<Frame> top;

        /** @brief Singleton constructor */
        SymbolTable();
    public:
        ~SymbolTable();

        /** @brief Global environment */
        static Environment *global();

        /** @brief Add symbol table to top of stack */
        static void push(Environment *env);

        /** @brief Remove top of stack */
        static void pop();

        /** @brief Adds a new envirnoment to the current top */
        static void use(Environment *env);

        /** @return Resolved path to symbol from any of the currently accessable symbol tables */
        static ResolutionPath resolve(parsers::BrainfuckLangParser::QualifiedIdContext *id);

        /** @return Resolved Symbol in any reachable scope or null */
        static ResolutionPath resolve(parsers::BrainfuckLangParser::UnqualifiedIdContext *id);

        /** @return Resolved Symbol in any reachable scope or null */
        static ResolutionPath resolve(id_t id, antlr4::ParserRuleContext *locationContext);

        /**
         * @tparam SymbolType
         * @return True if top can be cast to symbol type
         */
        template<typename SymbolType>
        static bool topIs() {
            return !top.empty() && top.back().strongScope->is<SymbolType>();
        }

        /**
         * @brief Top as a specific symbol type
         * @tparam SymbolType Expected type
         * @return Top symbol table cast to the specified type
         */
        template<typename SymbolType>
        static SymbolType *topAs() {
            return top.empty() ? nullptr : top.back().strongScope->as<SymbolType>();
        }

        /**
         * @return Current top symbol table
         */
        static Environment *getTop() {
            return top.empty() ? nullptr : top.back().strongScope;
        }

        /**
         * Cases:
         * 1. Simple id & declaration for id found: Return already existing symbol
         * 2. Simple id & declaration for id not found: Create new symbol and return that
         * 3. Nested id: Return existing symbol else throw
         * @param ctx Declaration Context
         * @param body Optional function implementation body
         * @return Symbol
         */
        static ResolutionPath declare(parsers::BrainfuckLangParser::FunctionDeclarationContext *ctx,
                               parsers::BrainfuckLangParser::FunctionImplementationContext *impl);

        /**
         * Cases:
         * 1. Simple id & declaration for id found: Return already existing symbol
         * 2. Simple id & declaration for id not found: Create new symbol and return that
         * 3. Nested id: Return existing symbol else throw
         * @param ctx Declaration Context
         * @param body Optional class body
         * @return Symbol
         */
        static ResolutionPath declare(parsers::BrainfuckLangParser::ClassDeclarationContext *ctx,
                               parsers::BrainfuckLangParser::ClassImplementationContext *body);

        /**
         * @brief Creates a new block scope
         * @param pContext Scope
         * @return Scope symbol
         */
        static Environment *declareBlockScope(parsers::BrainfuckLangParser::BlockStatementContext *pContext);
    };
}


#endif //BRAINFUCKLANG_SYMBOLTABLE_H
