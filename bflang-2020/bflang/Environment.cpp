//
// Created by Marian Plivelic on 09.04.18.
//
#include "Environment.h"
#include "Symbol.h"
#include "ClassSymbol.h"
#include "VariableSymbol.h"
#include "FunctionSymbol.h"
#include "CompilerException.h"
#include "Compiler.h"

namespace brainfuck::compiler {
    /** @brief static frame top */
    std::vector<SymbolTable::Frame> SymbolTable::top = { std::move(Frame(SymbolTable::global())) };

    Environment::Environment() = default;

    Environment::~Environment() = default;

    ResolutionPath Environment::resolve(parsers::BrainfuckLangParser::QualifiedIdContext *ctx) {
        ResolutionPath path{};

        // get id vector
        auto ids = ctx->unqualifiedId();

        // resolve initial
        std::vector<Symbol*> matches = resolve(ids.front()).matching;

        for (auto it = ids.begin() + 1; it != ids.end(); ++it) {
            // must exist
            if (matches.empty())
                exceptions::emit(*(it - 1), "Undefined symbol " + (*(it - 1))->getText());

            // match must be unique
            if (matches.size() > 1)
                exceptions::emit(*(it - 1), "Ambiguous symbol " + (*(it - 1))->getText());

            // get unique scope symbol
            Symbol *match = matches.front();

            auto table = dynamic_cast<Environment *>(match);
            if (table == nullptr) {
                // scope must be a symbol table
                exceptions::emit(*(it - 1), (*(it - 1))->getText() + " does not contain any accessable symbols");
            } else if (table->is<FunctionSymbol>()) {
                // scope may not be a function
                exceptions::emit(*(it - 1), "Scope may not be a function: " + table->qualifiedId());
            } else {
                // add the symbol table to the path
                path.matching.push_back(table);

                // resolve the next element
                assert(table);
                matches = table->resolve(*it).matching;
            }
        }

        // set result matches
        path.matching = matches;

        // set result unique symbol
        path.resolvedSymbol = matches.size() == 1 ? path.matching.front() : nullptr;

        return path;
    }

    ResolutionPath Environment::resolve(id_t id) {
        ResolutionPath path{};

        // get matching symbols
        auto range = children.equal_range(id);

        // extract symbols from range and return
        path.matching.reserve(static_cast<unsigned long>(std::distance(range.first, range.second)));
        for (auto it = range.first; it != range.second; ++it)
            path.matching.push_back(it->second.get());

        // set unique symbol
        path.resolvedSymbol = path.matching.size() == 1 ? path.matching.front() : nullptr;
        return path;
    }

    ResolutionPath Environment::resolve(parsers::BrainfuckLangParser::UnqualifiedIdContext *ctx) {
        ResolutionPath path;
        if (ctx->unqualifiedPrimaryId()) {
            path  = resolve(ctx->unqualifiedPrimaryId()->getText());
        } else if (ctx->unqualifiedOperatorId()) {
            if (ctx->unqualifiedOperatorId()->anyOperator()) {
                // todo
            } else if (ctx->unqualifiedOperatorId()->qualifiedId()) {
                // todo
            }
        } else if (ctx->unqualifiedTemplateId()) {
            // todo
            path = resolve(ctx->unqualifiedTemplateId()->IDENTIFIER()->getText());
            // non unique path if template already initialized
            // select unique path by resolving the template arguments
        }
        return path;
    }

    Symbol *Environment::create(id_t id, std::unique_ptr<Symbol> symbol, void *anonymous) {
        // save return ptr
        Symbol *ptr = symbol.get();
        // set initial values
        symbol->id = id;
        symbol->isAnonymous = anonymous != nullptr;
        symbol->parent = this;
        if (anonymous) {
            // add to anonymous symbols
            assert(this->anonymousSymbols.count(anonymous) == 0);
            this->anonymousSymbols.insert({anonymous, std::move(symbol)});
        } else {
            // insert into children multimap
            this->children.insert(std::make_pair(id, std::move(symbol)));
        }
        return ptr;
    }

    Symbol *Environment::create(parsers::BrainfuckLangParser::UnqualifiedIdContext *ctx, std::unique_ptr<Symbol> symbol,
                                void *anonymous) {
        if (ctx->unqualifiedPrimaryId()) {
            return create(ctx->getText(), std::move(symbol), anonymous);
        } else if (ctx->unqualifiedOperatorId())
            exceptions::emit(ctx, "Template unsupported");
        else if (ctx->unqualifiedOperatorId())
            exceptions::emit(ctx, "Operator unsupported");
        return nullptr;
    }

    VariableSymbol *Environment::declare(parsers::BrainfuckLangParser::VariableDeclarationContext *ctx, const VariableQualifiers &qualifiers) {
        VariableSymbol *variable = nullptr;
        if (ctx->IDENTIFIER()) {
            // resolve identifier
            auto old = resolve(ctx->IDENTIFIER()->getText()).matching;

            // prevent redefinition
            if (!old.empty())
                exceptions::emit(ctx, "Redefinition of " + ctx->IDENTIFIER()->getText() + " previously defined at " +
                                      std::string(old.front()->firstDeclaration()));

            // create named variable
            variable = create(ctx->IDENTIFIER()->getText(), std::make_unique<VariableSymbol>())->as<VariableSymbol>();
        } else {
            // create new anonymous variable
            variable = create("<anonymous variable>", std::make_unique<VariableSymbol>(), ctx)->as<VariableSymbol>();
        }

        // add new declaration location
        variable->declarationLocations.emplace_back(Compiler::currentFile(), ctx);

        // set variable type
        variable->type = std::move(Type::create(ctx->typeSignature()));

        // set specified type qualifiers
        variable->type->qualifiers.hasConstexpr = qualifiers.hasConstexpr;
        variable->type->qualifiers.hasConst = qualifiers.hasConst;

        // set variable qualifiers
        variable->qualifiers = qualifiers;

        // set variable static if parent is global scope
        variable->qualifiers.hasStatic |= variable->parent == SymbolTable::global();

        // set symbol completed
        variable->setCompleted(ctx);

        if (variable->parent->is<ClassSymbol>() && variable->parent->as<ClassSymbol>()->isInterface && !variable->qualifiers.hasStatic)
            exceptions::emit(ctx, "Variables can't be declared inside of interfaces");

        if (variable->qualifiers.hasStatic) {
            // static variables update the global offset
            variable->address = SymbolTable::global()->offset;
            SymbolTable::global()->offset += variable->type->getSize(ctx);
        } else {
            // else update the local offset
            variable->address = variable->parent->offset;
            variable->parent->offset += variable->type->getSize(ctx);
            // todo: convert local offset -> parent offset if block scope
            // create Environment::updateOffset() -> FunctionSymbol::updateOffset() override
        }

        return variable;
    }

    ClassSymbol *Environment::declare(parsers::BrainfuckLangParser::ClassDeclarationContext *ctx) {
        Symbol *symbol;

        if (ctx->qualifiedId()) {
            // find old forward declaration in current scope
            symbol = resolve(ctx->qualifiedId()).resolvedSymbol;

            if (!symbol) {
                // create new symbol if not found
                symbol = create(ctx->qualifiedId()->getText(), std::make_unique<ClassSymbol>());
            } else if (!symbol->is<ClassSymbol>()
                       || (symbol->as<ClassSymbol>()->isInterface != (ctx->isInterface != nullptr))) {
                // error if not class symbol or not an interface if previously declared as one
                exceptions::emit(ctx, "Symbol type mismatch for " + symbol->qualifiedId() + " declared at " +
                                      std::string(symbol->firstDeclaration()));
            }
        } else {
            // new anonymous struct
            symbol = create("<anonymous class>", std::make_unique<ClassSymbol>(), ctx);
        }

        // interface flag set
        symbol->as<ClassSymbol>()->isInterface = ctx->isInterface != nullptr;

        // Remember declaration location
        symbol->declarationLocations.emplace_back(Compiler::currentFile(), ctx);

        return dynamic_cast<ClassSymbol *>(symbol);
    }

    FunctionSymbol *Environment::declare(parsers::BrainfuckLangParser::FunctionDeclarationContext *ctx) {
        // path for scope
        auto path = resolve(ctx->qualifiedId());

        // if a symbol exists, it has to be a function
        if (!path.matching.empty() && !path.allOf<FunctionSymbol>())
            exceptions::emit(ctx, "Function expected: " + ctx->qualifiedId()->getText());

        // parse signature
        auto signature = std::make_unique<FunctionSignature>(ctx->functionSignature());

        // find matching function signature
        FunctionSymbol *symbol = path.findWithSignature(signature.get());

        if (!symbol) {
            // create new function symbol if not found
            symbol = create(ctx->qualifiedId()->getText(), std::make_unique<FunctionSymbol>())->as<FunctionSymbol>();
            // add signature
            symbol->signature = std::move(signature);
            // add specifiers
            symbol->specifiers = FunctionSpecifiers(ctx->functionSpecifier());
        } else {
            if (*signature != *symbol->as<FunctionSymbol>()->signature) {
                // error if symbol already existed but signatures don't match
                std::stringstream ss;
                ss << "Function signature mismatch: " << *signature << " vs "
                   << *symbol->as<FunctionSymbol>()->signature;
                exceptions::emit(ctx->functionSignature(), ss.str());
            } else if (!symbol->as<FunctionSymbol>()->signature->equalReturnTypes(signature.get())) {
                exceptions::emit(ctx, "Function declarations can't only be different in return type. Previously defined as " + symbol->as<FunctionSymbol>()->qualifiedId());
            }
        }

        // Remember declaration location
        symbol->declarationLocations.emplace_back(Compiler::currentFile(), ctx);

        // Set member flag
        symbol->as<FunctionSymbol>()->isMember = symbol->parent->is<ClassSymbol>();

        // set special member flag
        if (!symbol->isAnonymous) {
            if (symbol->id.at(0) == '~') {
                // error if destructor outside of class
                if (!symbol->parent || !symbol->parent->is<ClassSymbol>() || symbol->parent->as<ClassSymbol>()->isInterface)
                    exceptions::emit(ctx, "Only classes can have destructors");
                // error if destructor id doesn't match the parent class id
                if (symbol->id != '~' + symbol->parent->id)
                    exceptions::emit(ctx, "Unrelated class destructor declaration");
                if (!symbol->signature->returnTypes.empty() || !symbol->signature->parameterTypes.empty())
                    exceptions::emit(ctx, "Destructors may not have parameters and no return type");
                symbol->isDestructor = true;
            } else if (symbol->parent && symbol->id == symbol->parent->id) {
                // error if outside of class
                if (!symbol->parent->is<ClassSymbol>() || symbol->parent->as<ClassSymbol>()->isInterface)
                    exceptions::emit(ctx, "Only classes can have constructors");
                symbol->isConstructor = true;
            }
        }

        return symbol;
    }

    ClassSymbol *Environment::resolve(parsers::BrainfuckLangParser::ClassDeclarationContext *anonymousClassDeclaration) {
        if (anonymousClassDeclaration->qualifiedId() != nullptr)
            exceptions::emit(anonymousClassDeclaration->qualifiedId(), "Anonymous class expected");
        if (anonymousSymbols.count(anonymousClassDeclaration) == 0)
            exceptions::emit(anonymousClassDeclaration, "Anonymous class not found");
        if (!anonymousSymbols.at(anonymousClassDeclaration)->is<ClassSymbol>())
            exceptions::emit(anonymousClassDeclaration, "Symbol type mismatched. A class was expected.");
        return anonymousSymbols.at(anonymousClassDeclaration)->as<ClassSymbol>();
    }

    ResolutionPath SymbolTable::declare(parsers::BrainfuckLangParser::FunctionDeclarationContext *ctx,
                                             parsers::BrainfuckLangParser::FunctionImplementationContext *impl) {
        ResolutionPath path{};

        // forward declaration on top
        if (ctx->qualifiedId()->unqualifiedId().size() <= 1) {
            path.resolvedSymbol = getTop()->declare(ctx);
        } else {
            // find previous symbol declaration
            path = resolve(ctx->qualifiedId());

            // previous declaration has to be a function
            if (!path.allOf<FunctionSymbol>()) {
                exceptions::emit(ctx, "Out-of-scope redeclaration of " + path.matching.front()->id + " defined at " +
                                      std::string(path.matching.front()->firstDeclaration()));
            }

            // find with matching signature
            FunctionSignature signature(ctx->functionSignature());
            FunctionSymbol *function = path.findWithSignature(&signature);

            // check forward declaration exists
            if (function == nullptr) {
                std::stringstream ss;
                ss << "Declaration for function '" << ctx->qualifiedId()->getText() << signature << "' not found";
                exceptions::emit(ctx, ss.str());
            }

            if (!function->signature->equalReturnTypes(&signature)) {
                std::stringstream ss;
                ss << "Function return types do not match: '" << *function->signature << "' vs '" << signature << "'";
                exceptions::emit(ctx, ss.str());
            }

            if (!impl) {
                std::stringstream ss;
                ss << "Out-of-scope function declaration of '" << function->qualifiedId() <<
                   signature << "' requires an implementation";
                exceptions::emit(ctx, ss.str());
            }

            if (FunctionSpecifiers(ctx->functionSpecifier()) != FunctionSpecifiers()) {
                exceptions::emit(ctx, "Out-of-scope declarations do not accept function specifiers");
            }
        }

        return path;
    }

    ResolutionPath SymbolTable::declare(parsers::BrainfuckLangParser::ClassDeclarationContext *ctx,
                                             parsers::BrainfuckLangParser::ClassImplementationContext *body) {
        ResolutionPath path{};
        if (ctx->qualifiedId() == nullptr || ctx->qualifiedId()->unqualifiedId().size() <= 1) {
            // forward declaration on top
            path.resolvedSymbol = getTop()->declare(ctx);
        } else {
            // find previous declaration
            path = resolve(ctx->qualifiedId());

            // check forward declaration exists
            if (path.resolvedSymbol == nullptr)
                exceptions::emit(ctx, "Declaration for " + ctx->qualifiedId()->getText() + " not found");

            // previous declaration has to be a class
            if (!path.resolvedSymbol->is<ClassSymbol>())
                exceptions::emit(ctx,
                                 "Out-of-scope symbol type mismatch. Class or structure expected. Was previously declared here: " +
                                 std::string(path.resolvedSymbol->firstDeclaration()));
        }

        // error on anonymous without implementation
        if (!body && path.resolvedSymbol->isAnonymous)
            exceptions::emit(ctx, "Anonymous declaration class or interface requires an in-place implementation");

        // error on reimplementation
        if (body && path.resolvedSymbol->isComplete)
            exceptions::emit(ctx, "Redefinition of class or interface " + path.resolvedSymbol->qualifiedId() + " defined at " +
                                  std::string(path.resolvedSymbol->implementationLocation));

        // interface flag set
        path.resolvedSymbol->as<ClassSymbol>()->isInterface = ctx->isInterface != nullptr;

        return path;
    }

    void SymbolTable::push(Environment *env) {
        top.push_back(std::move(Frame(env)));
    }

    void SymbolTable::pop() {
        top.pop_back();
    }

    void SymbolTable::use(Environment *env) {
        top.back().weakScopes.push_back(env);
    }

    SymbolTable::SymbolTable() {
        // push global scope
        top.push_back(std::move(Frame(new Environment)));
    }

    ResolutionPath SymbolTable::resolve(id_t id, antlr4::ParserRuleContext *locationContext) {
        // find in current scope and parents
        Environment *env = top.back().strongScope;
        do {
            auto path = env->resolve(id);
            if (!path.matching.empty())
                return path;
            env = env->parent;
        } while (env != nullptr);

        // find in weak scopes on stack
        for (auto it = top.rbegin(); it != top.rend(); ++it) {
            for (auto weak : it->weakScopes) {
                auto path = weak->resolve(id);
                if (!path.matching.empty())
                    return path;
            }
        }

        return ResolutionPath();
    }

    ResolutionPath SymbolTable::resolve(parsers::BrainfuckLangParser::UnqualifiedIdContext *id) {
        if (id->unqualifiedPrimaryId())
            return resolve(id->unqualifiedPrimaryId()->getText(), id);
        else if (id->unqualifiedOperatorId())
            exceptions::emit(id, "Operator id not supported");
        else if (id->unqualifiedTemplateId())
            exceptions::emit(id, "Template id not supported");
        return ResolutionPath();
    }

    ResolutionPath SymbolTable::resolve(parsers::BrainfuckLangParser::QualifiedIdContext *id) {
        // get id object
        auto ids = id->unqualifiedId();
        // get id of initial symbol
        auto uid = ids.front();
        // resolve initial symbol
        ResolutionPath initial = resolve(uid);
        // check initial is unique
        if (initial.matching.size() > 1)
            exceptions::emit(id, "Non unique symbol id " + id->getText());
        // check initial found
        if (!initial.resolvedSymbol)
            exceptions::emit(id, "Undefined symbol id " + id->getText());
        auto table = initial.resolvedSymbol->parent;
        return table->resolve(id);
    }

    Environment *SymbolTable::declareBlockScope(parsers::BrainfuckLangParser::BlockStatementContext *pContext) {
        return getTop()->create("<block scope>", std::move(std::unique_ptr<Symbol>(new Environment)), pContext)->as<Environment>();
    }

    Environment *SymbolTable::global() {
        static Environment *global = new Environment;
        return global;
    }

    SymbolTable::~SymbolTable() = default;

    FunctionSymbol *ResolutionPath::findWithSignature(FunctionSignature *signature) {
        if (resolvedSymbol) {
            if (resolvedSymbol->is<FunctionSymbol>() && *resolvedSymbol->as<FunctionSymbol>()->signature == *signature)
                return resolvedSymbol->as<FunctionSymbol>();
        } else {
            for (auto f : matching) {
                if (f->is<FunctionSymbol>() && *f->as<FunctionSymbol>()->signature == *signature)
                    return f->as<FunctionSymbol>();
            }
        }
        return nullptr;
    }
}
