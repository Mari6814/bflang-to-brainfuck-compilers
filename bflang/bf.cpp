#include "bf.h"
#include "print.h"
#include <assert.h>
#include <sstream>

namespace {
    int jumpAddressCounter = 0;

    std::string parseStringEscape(const std::string &that) {
        std::string str;

        for (int i = 0; i < that.size(); i++) {
            if (i + 1 < that.size() && that[i] == '\\') {
                switch (that[++i]) {
                    case 'n': str += '\n'; continue;
                    case 'r': str += '\r'; continue;
                    case 't': str += '\t'; continue;
                    case '\\': str += '\\'; continue;
                    default:
                        str += that[i--];
                }
                // todo: parse \000 and \xHH
            } else {
                str += that[i];
            }
        }
        return str;
    }


    string joinQualified(const QualifiedName &qn) {
        if (qn.size() == 0)
            return "";

        std::stringstream ss;

        for (int i = 0; i < qn.size() - 1; i++)
            ss << qn[i] << '.';
        ss << qn.back();

        return ss.str();
    }

    string symbol2str(const Symbol& symbol) {
        string base;
        if (dynamic_cast<const VariableSymbol*>(&symbol))
            base = "Variable";
        if (dynamic_cast<const FunctionSymbol*>(&symbol))
            base = "Function";
        if (dynamic_cast<const TypeSymbol*>(&symbol))
            base = "Type";
        if (dynamic_cast<const StackframeSymbol*>(&symbol))
            base = "Stackframe";
        return base + (symbol.temp ? "* " : " " ) + joinQualified(symbol.getQualified());
    }

    // returns the index of the maximum shared path between the end of lhs and beginning of rhs
    size_t sharedPathSize(const QualifiedName &lhs, const QualifiedName &rhs) {
        QualifiedName tmp;
        tmp.reserve(rhs.size());
        for (size_t i = 1; i <= lhs.size() && i <= rhs.size(); ++i) {
            tmp.push_back(rhs[i - 1]);
            if (QualifiedName(lhs.end() - i, lhs.end()) == tmp)
                return i;
        }
        return 0;
    }

    string result2str(const string &file, int line, const SymbolResolutionResult &result) {
        auto begin = result.dereference(file, line);
        auto size = result.resolved->getSizeOnTheStack();
        return result.name() + "@" + to_string(begin) + ":" + to_string(size);
    }

    template<typename... Args>
    inline void die(string file, int line, int error, const Args&... args) {
#ifdef EXIT_ON_DIE
        errprintln(file + ":" + to_string(line), "Error", to_string(error) + ":", args...);
        exit(error);
#else
        stringstream ss;
        osprint(ss, file + ":" + to_string(line), "Error", to_string(error) + ":", args...);
        throw std::runtime_error(ss.str());
#endif
    }

    size_t getSymbolVectorOverlap(const vector<const Symbol*> &lhs, const vector<const Symbol*> &rhs) {
        size_t i = 0;
        for (; i <= lhs.size() && i <= rhs.size(); i++) {
            bool flag = i > 0;
            for (size_t j = 0; j < i && flag; j++)
                flag &= lhs[lhs.size() - i + j] == rhs[j];
            if (flag)
                return i;
        }
        return 0;
    }

    FunctionSymbol *asFunction(const string &file, int line, SymbolResolutionResult &res) {
        auto fun = dynamic_cast<FunctionSymbol*>(res.resolved);
        if (!fun)
            die(file, line, EXIT_FAILURE, res.getTrace(), "Expected function but received", *res.resolved);
        return fun;
    }

    const VariableSymbol *asVariable(const string &file, int line, const SymbolResolutionResult &res) {
        if (!res)
            die(file, line, EXIT_FAILURE, res.getTrace(), "Unresolved variable");
        auto var = dynamic_cast<VariableSymbol*>(res.resolved);
        if (!var)
            die(file, line, EXIT_FAILURE, res.getTrace(), "Expected variable, but received", *res.resolved);
        return var;
    }

    string dereference(const string &file, int line, const SymbolResolutionResult &res) {
        auto asvar = asVariable(file, line, res);
        string position = to_string(res.dereference(file, line)) + ":" + to_string(asvar->getSizeOnTheStack());
        return asvar->name + (asvar->temp ? "*" : "") + "@" + position;
    }

    void checkType(const string &file, int line, const VariableSymbol *v1, const VariableSymbol *v2) {
        if (v1->isPointerType != v2->isPointerType)
            die(file, line, EXIT_FAILURE, "Type mismatch between pointer type and non-pointer type", v1->name, "and", v2->name);
        if (v1->type != v2->type || v1->length != v2->length)
            die(file, line, EXIT_FAILURE, "Type mismatch between", v1->name, "of type", v1->type2str(), "and", v2->name, "of type", v2->type2str());
    }

    void checkType(const string &file, int line, const SymbolResolutionResult &lhs, const SymbolResolutionResult &rhs) {
        checkType(file, line, asVariable(file, line, lhs), asVariable(file, line, rhs));
    }

    void checkType(const string &file, int line, const SymbolResolutionResult &var, const Symbol *type) {
        if (asVariable(file, line, var)->type != type)
            die(file, line, EXIT_FAILURE, "Expected variable of type", *type, "but received", *var.resolved);
    }

    string binop2str(BinaryOperatorExpression::OperatorType op) {
        switch (op) {
            case BinaryOperatorExpression::OperatorType::OP_MOV: return "MOV";
            case BinaryOperatorExpression::OperatorType::OP_ADD: return "ADD";
            case BinaryOperatorExpression::OperatorType::OP_SUB: return "SUB";
            case BinaryOperatorExpression::OperatorType::OP_MUL: return "MUL";
            case BinaryOperatorExpression::OperatorType::OP_DIV: return "DIV";
            default:
                return "";
        }
    }

    std::ostream &movePtr(std::ostream &os, cellValue offset) {
        return os << std::string(size_t(offset < 0 ? -offset : offset),  offset >= 0 ? '>' : '<');
    }

    std::ostream &insertAt(std::ostream &os, cellReference dst, std::string str) {
        movePtr(os, dst) << str;
        return movePtr(os, -dst);
    }

    std::ostream &zero(std::ostream &os, cellReference dst, cellSize size) {
        for (int i = 0; i < size; i++)
            insertAt(os, dst + i, "[-]");
        return os;
    }

    std::ostream &inc(std::ostream &os, cellReference dst) {
        return insertAt(os, dst, "+");
    }

    std::ostream &dec(std::ostream &os, cellReference dst) {
        return insertAt(os, dst, "-");
    }

    std::ostream &iadd(std::ostream &os, cellReference dst, cellValue value, cellSize size) {
        assert(size == 1);
        assert(dst >= 0);
        insertAt(os, dst, std::string((unsigned long) (value < 0 ? -value : value), value < 0 ? '-' : '+'));
        return os;
    }

    std::ostream &add(std::ostream &os, cellReference dst, cellReference src, cellSize size) {
        for (int i = 0; i < size; i++) {
            insertAt(os, src + i, "[");
            dec(os, src + i);
            inc(os, dst + i);
            insertAt(os, src + i, "]");
        }
        return os;
    }

    std::ostream &add(std::ostream &os, cellReference dst, cellReference src, cellReference aux, cellSize size) {
        zero(os, aux, 1);
        for (int i = 0; i < size; i++) {
            insertAt(os, src + i, "[");
            dec(os, src + i);
            inc(os, dst + i);
            inc(os, aux);
            insertAt(os, src + i, "]");
            add(os, src + i, aux, 1);
        }
        return os;
    }

    std::ostream &sub(std::ostream &os, cellReference dst, cellReference src, cellSize size) {
        for (int i = 0; i < size; i++) {
            insertAt(os, src + i, "[");
            dec(os, src + i);
            dec(os, dst + i);
            insertAt(os, src + i, "]");
        }
        return os;
    }

    std::ostream &sub(std::ostream &os, cellReference dst, cellReference src, cellReference aux, cellSize size) {
        zero(os, aux, 1);
        for (int i = 0; i < size; i++) {
            insertAt(os, src + i, "[");
            dec(os, src + i);
            dec(os, dst + i);
            inc(os, aux);
            insertAt(os, src + i, "]");
            add(os, src + i, aux, 1);
        }
        return os;
    }

    std::ostream &foreach(std::ostream &os, cellReference dst, cellSize size, std::string op) {
        for (int i = 0; i < size; ++i)
            insertAt(os, dst + i, op);
        return os;
    }

    void outputInstruction(Instruction &i) {
        osprintln(intermediate_output_stream, i);
        i.release = true;
        osprintln(binary_output_stream, i);
    }

    void outputIntegerInstruction(const std::string &file, int line,
                                  BinaryOperatorExpression::OperatorType op,
                                  const SymbolResolutionResult &lhs, cellValue integer) {
        InstructionName instructionName = InstructionName::UNINITIALIZED;
        switch (op) {
            case BinaryOperatorExpression::OP_MOV:
                instructionName = InstructionName::ILOAD; break;
            case BinaryOperatorExpression::OP_ADD:
                instructionName = InstructionName::IADD; break;
            case BinaryOperatorExpression::OP_SUB:
                instructionName = InstructionName::ISUB; break;
            default:
                die(file, line, EXIT_FAILURE, "Invalid operation");
        }
        Instruction i(file, line, instructionName, dereference(file, line, lhs) + " " + to_string(integer));
        i.constant.dst = (int) lhs.dereference(file, line);
        i.constant.value = instructionName == InstructionName::ISUB ? -integer : integer;
        i.constant.size = (int) lhs.resolved->getSizeOnTheStack();
        if (i.constant.size != 1)
            die(file, line, EXIT_SUCCESS, "Invalid assign of integer to variable of size", i.constant.size);
        outputInstruction(i);
    }

    void outputIoInstruction(const std::string &file, int line, IOStatement::IOFunction function,
                             const SymbolResolutionResult &dst) {
        Instruction i(file, line, function == IOStatement::IOFunction::IOINPUT ? InstructionName::WRITE_INPUT
                                                                               : InstructionName::WRITE_OUTPUT,
                      result2str(file, line, dst));
        i.io.src = (int) dst.dereference(file, line);
        i.io.size = (int) dst.resolved->getSizeOnTheStack();
        outputInstruction(i);
    }

    void outputMoveInstruction(const std::string &file, int line, BinaryOperatorExpression::OperatorType op,
                               const SymbolResolutionResult &lhs, const SymbolResolutionResult &rhs) {
        if (lhs != rhs) {
            checkType(file, line, lhs, rhs);
            InstructionName instructionName = InstructionName::UNINITIALIZED;
            switch (op) {
                case BinaryOperatorExpression::OP_MOV:
                    instructionName = InstructionName::MOVE; break;
                case BinaryOperatorExpression::OP_ADD:
                    instructionName = InstructionName::ADD; break;
                case BinaryOperatorExpression::OP_SUB:
                    instructionName = InstructionName::SUB; break;
                default:
                    die(file, line, EXIT_FAILURE, "Invalid operation");
            }
            Instruction i(file, line, instructionName,
                          dereference(file, line, lhs) + " " + dereference(file, line, rhs));
            i.move.dst = (int) lhs.dereference(file, line);
            i.move.src = (int) rhs.dereference(file, line);
            i.move.size = (int) lhs.resolved->getSizeOnTheStack();
            outputInstruction(i);
        }
    }

    void outputCopyInstruction(const std::string &file, int line, BinaryOperatorExpression::OperatorType op,
                               const SymbolResolutionResult &lhs, const SymbolResolutionResult &rhs,
                               const SymbolResolutionResult &aux) {
        if (lhs != rhs) {
            InstructionName instructionName = InstructionName::UNINITIALIZED;
            switch (op) {
                case BinaryOperatorExpression::OP_MOV:
                    instructionName = InstructionName::COPY; break;
                case BinaryOperatorExpression::OP_ADD:
                    instructionName = InstructionName::ADD_COPY; break;
                case BinaryOperatorExpression::OP_SUB:
                    instructionName = InstructionName::SUB_COPY; break;
                default:
                    die(file, line, EXIT_FAILURE, "Invalid operation");
            }
            Instruction i(file, line, instructionName,
                          dereference(file, line, lhs) + " " + dereference(file, line, rhs) + " " + dereference(file, line, aux));
            i.copy.dst = (int) lhs.dereference(file, line);
            i.copy.src = (int) rhs.dereference(file, line);
            i.copy.aux = (int) aux.dereference(file, line);
            i.copy.size = (int) lhs.resolved->getSizeOnTheStack();
            i.copy.size_aux = (int) aux.resolved->getSizeOnTheStack();
            outputInstruction(i);
        }
    }

    void outputCopyInstruction(const std::string &file, int line, cellReference lhs, cellReference rhs, cellReference aux) {
        Instruction i(file, line, InstructionName::COPY, to_string(lhs) + ", " + to_string(rhs) + ", " + to_string(aux));
        i.copy.aux = aux;
        i.copy.size_aux = 1;
        i.copy.dst = lhs;
        i.copy.src = rhs;
        i.copy.size = 1;
        outputInstruction(i);
    }

    void outputAutoMoveInstruction(const std::string &file, int line, SymbolTable &symbolTable,
                                   BinaryOperatorExpression::OperatorType op, const SymbolResolutionResult &lhs,
                                   const SymbolResolutionResult &rhs) {
        if (lhs != rhs) {
            checkType(file, line, lhs, rhs);
            if (rhs.resolved->temp) {
                // rhs is temporary and can be safely destroyed
                outputMoveInstruction(file, line, op, lhs, rhs);
            } else {
                // todo: pop old stackframe (because rhs is not a tmp, it can't be contained in that scope), create new stack, add variable, pop?
                symbolTable.push(*symbolTable.newTmpStackframe(line));
                auto aux = symbolTable.newTmpVariable(line, asVariable(file, line, lhs)->type);
                outputCopyInstruction(file, line, op, lhs, rhs, aux);
                symbolTable.pop();
            }
        }
    }

    void outputStackInstruction(const std::string &file, int line, bool isPop, cellValue offset, std::string comment = "") {
        Instruction i(file, line, isPop ? InstructionName::POP_STACK : InstructionName::PUSH_STACK, comment);
        i.stack.offset = offset;
        if (offset != 0)
            outputInstruction(i);
    }

    void outputLabelInstruction(const std::string &file, int line, cellValue offset, label label, std::string comment) {
        Instruction i(file, line, InstructionName::LABEL, comment + "@" + to_string(label));
        i.label.address = label;
        outputInstruction(i);
        outputStackInstruction(file, line, true, offset);
    }

    void outputCompareInstruction(const std::string &file, int line, cellReference isZero, cellReference notZero, cellReference condition, cellSize size) {
        if (size != 1)
            die(file, line, EXIT_FAILURE, "Only size 1 variable can be a condition");
        Instruction i(file, line, InstructionName::COMPARE);
        i.compare.conditionAddress = condition;
        i.compare.isZero = isZero;
        i.compare.notZero = notZero;
        i.comment = "cond@" + to_string(i.compare.conditionAddress) + ", isZero@" + to_string(isZero) + ", notZero@" + to_string(notZero);
        outputInstruction(i);
    }

    void outputTestInstruction(const std::string &file, int line, const SymbolResolutionResult &condition, cellReference jumpRegister, label onTrue, label onFalse) {
        Instruction i(file, line, InstructionName::TEST, "truebr@" + to_string(onTrue) + ", " + "falsebr@" + to_string(onFalse) + ", jmpreg@" + to_string(jumpRegister));
        i.test.jumpRegister = jumpRegister;
        i.test.isTrue = jumpRegister + 1;
        i.test.isFalse = jumpRegister + 2;
        i.test.trueLabel = onTrue;
        i.test.falseLabel = onFalse;

        if (condition.resolved->getSizeOnTheStack() != 1)
            die(file, line, EXIT_FAILURE, "Condition not of size 1");

        if (condition.resolved->temp) {
            outputCompareInstruction(file, line, i.test.isFalse, i.test.isTrue, (int) condition.dereference(file, line), 1);
        } else {
            auto aux = jumpRegister + 3;
            outputCopyInstruction(file, line, aux, static_cast<int>(condition.dereference(file, line)), i.test.isTrue);
            outputCompareInstruction(file, line, i.test.isFalse, i.test.isTrue, aux, 1);
        }
        outputInstruction(i);
    }

    void outputJumpInstruction(const std::string &file, int line, cellValue offset, label address, std::string comment = "") {
        Instruction i(file, line, InstructionName::JUMP, std::move(comment));
        i.jump.targetAddress = address;
        outputStackInstruction(file, line, false, offset);
        outputInstruction(i);
    }

    void outputLoadStringInstruction(const std::string &file, int line, const SymbolResolutionResult& dst, const std::string &str) {
        auto adr = dst.dereference(file, line);
        for (int i = 0; i < str.size(); i++) {
            Instruction instr(file, line, InstructionName::ILOAD, result2str(file, line, dst) + "+" + to_string(i) + ", " + to_string((int)str[i]));
            instr.constant.size = 1;
            instr.constant.dst = static_cast<cellReference>(adr + i);
            instr.constant.value = str[i];
            outputInstruction(instr);
        }
    }
}

ostream &operator<<(ostream &os, const Symbol &symbol) {
    os << symbol2str(symbol);

    auto fun = dynamic_cast<const FunctionSymbol*>(&symbol);
    if (fun != nullptr)
        os << " address=" << fun->address;
    else {
        auto var = dynamic_cast<const VariableSymbol*>(&symbol);
        if (var != nullptr) {
            os << " address=" << var->getAddressRelativeToFunctionStackframe() << " size=" << var->getSizeOnTheStack() << " length=" << var->length << " type=" << *var->type;
        }
        else {
            auto type = dynamic_cast<const TypeSymbol*>(&symbol);
            if (type != nullptr) {
                os << " size=" << type->getSizeSumOfChildSymbols();
            }
        }
    }
    return os;
}

ostream &operator<<(ostream &os, const SymbolTable &st) {
    vector<const Symbol*> queue;
    assert(st.scopeStack.size() == 1);
    for (auto s : st.scopeStack[0]->symbols) {
        queue.push_back(s);
        while (queue.size() > 0) {
            if (verboseSymbolTable) {
                os << *queue.front() << endl;
            } else {
                if (!queue.front()->temp)
                    os << *queue.front() << endl;
            }
            for (const Symbol *child : queue.front()->symbols)
                queue.push_back(child);
            queue.erase(queue.begin());
        }
    }
    return os << endl;
}

QualifiedName Symbol::getQualified() const {
    QualifiedName qn;
    if ((parent != nullptr) && parent->parent)
        qn = parent->getQualified();
    qn.push_back(name);
    return qn;
}

void Symbol::setParent(Symbol *newParent) {
    if (newParent == nullptr)
        die(file, line, EXIT_FAILURE, "Parent scope is null");

    if (parent != nullptr)
        die(file, line, EXIT_FAILURE, "Scope already set");
    parent = newParent;
    newParent->symbols.push_back(this);
}

vector<const Symbol *> Symbol::fullPath() const {
    vector<const Symbol*> path;
    if (parent != nullptr)
        path = parent->fullPath();
    path.push_back(this);
    return path;
}

const FunctionSymbol *Symbol::getParentFunctionStackframe() const {
    return dynamic_cast<const FunctionSymbol*>(this) != nullptr
           ? dynamic_cast<const FunctionSymbol*>(this)
           : (parent != nullptr ? parent->getParentFunctionStackframe() : nullptr);
}

SymbolTable::SymbolTable() {
    // initialize scope stack with the global scope
    auto root = new StackframeSymbol(0, "<init>", "__root__");
    push(*root);
}

Symbol* SymbolTable::currentScope() const {
    return scopeStack.back();
}

void SymbolTable::push(Symbol &symbol) {
    scopeStack.push_back(&symbol);
}

void SymbolTable::add(Symbol &symbol, bool temporary) {
    auto result = findGlobal(symbol.getQualified());
    if (result && result.resolved->parent == currentScope())
        die(symbol.file, symbol.line, EXIT_FAILURE, "Redifinition of " + symbol2str(symbol) + ", defined as " + symbol2str(*result.resolved) + " in line " + to_string(result.resolved->line) + ",");
    symbol.setParent(currentScope());
    symbol.temp = temporary;
}

void SymbolTable::pop() {
    scopeStack.pop_back();
    assert(scopeStack.back());
}

SymbolResolutionResult SymbolTable::findGlobal(QualifiedName qualified) {
    Symbol *scope = currentScope();
    // begin search in current scope, and reduce scope by one, each time
    // the desired qualified qualifiedName is not found
    while (scope) {
        SymbolResolutionResult resolution(scope);
        for (auto name : qualified) {
            resolution.find(name);
            if (!resolution)
                break;
        }
        // return if found
        if (resolution) {
            return resolution;
        }
        // else reduce scope by one, and try again
        scope = scope->parent;
    }
    return SymbolResolutionResult(nullptr);
}

SymbolResolutionResult SymbolTable::newTmpVariable(int line, TypeSymbol *type, int length, const char *debug_prefix) {
    static int tmpcount = 0;
    auto newvar = new VariableSymbol(line, currentFile, debug_prefix + to_string(tmpcount++), type);
    if (length > 0) {
        newvar->length = length;
        newvar->isPointerType = true;
    }
    add(*newvar, true);
    // todo: delete tmp variables on leaving scope
    // todo: don't bother with allocating memory, if I can just always use the end of the stack
    return findGlobal(QualifiedName{newvar->name});
}

StackframeSymbol *SymbolTable::newTmpStackframe(int line) {
    static int tmpcount = 0;
    auto frame = new StackframeSymbol(line, currentFile, "__frame" + to_string(tmpcount++));
    add(*frame, true);
    // todo: delete temporary stackframes on pop
    // todo: if no need for tmp variables -> don't make tmp stackframes
    return frame;
}

void SymbolTable::initFunctionStackframe(const std::string &file, int line, bool temporary) {
    add(*new VariableSymbol(line, file, "__ret", dynamic_cast<TypeSymbol *>(scopeStack[0]->symbols[0])), temporary);
}

void IntExpression::compile(CompilationState &state) {
    out = state.symbolTable.newTmpVariable(line, state.cellType);
    outputIntegerInstruction(file, line, BinaryOperatorExpression::OP_MOV, out, integer);
}

void IdentifierExpression::compile(CompilationState &state) {
    if (out)
        out = out.find(*identifier);
    else
        out = state.symbolTable.findGlobal(QualifiedName{*identifier});
    if (!out)
        die(file, line, EXIT_FAILURE, "Unresolved identifier", "'" + *identifier + "'");
}

IdentifierExpression::~IdentifierExpression() {
    delete identifier;
    identifier = nullptr;
}

IdentifierExpression::IdentifierExpression(string *identifier)
        : identifier(identifier) {}

void BinaryOperatorExpression::compile(CompilationState &state) {
    auto lhscall = dynamic_cast<CallExpression*>(lhs);
    auto rhscall = dynamic_cast<CallExpression*>(rhs);
    auto lhstuple = dynamic_cast<TupleExpression*>(lhs);
    auto rhstuple = dynamic_cast<TupleExpression*>(rhs);
    if (op == OP_ADD || op == OP_SUB) {
        if (rhstuple || lhstuple)
            die(file, line, EXIT_FAILURE, "Operator", binop2str(op),"not allowed on tuple expression");

        out = state.symbolTable.newTmpVariable(line, state.cellType);
        state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
        // todo: use lookahead for direct compilation
        // todo: lhs int expression can be statically compiled with operator ADD

        /*
         *  y = 1 - y; -> ILOAD aux 1;
         *                SUB aux y;
         *                MOV y aux;
         *  y = 1 - x; -> ILOAD y 1;
         *                SUB y x aux;
         *  x = x - 1; -> ISUB x 1;
         *  y = x - 1; -> CPY y x aux;
         *                ISUB y 1;
         */

        state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
        lhs->compile(state);
        checkType(file, line, out, lhs->out);

        if (lhscall && lhscall->returnValuesToPop.size() == 0)
            die(file, line, EXIT_FAILURE, "Function does not return a value");
        if (lhscall && lhscall->returnValuesToPop.size() > 1)
            errprintln(file + ":" + to_string(line), "Warning: Function returns more than one value. Using only the first return value.");

        outputAutoMoveInstruction(file, line, state.symbolTable, BinaryOperatorExpression::OP_MOV, out, lhs->out);
        state.symbolTable.pop();

        state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
        // statically compiles int expressions
        rhs->compile(state);
        checkType(file, line, out, rhs->out);
        if (rhscall && rhscall->returnValuesToPop.size() == 0)
            die(file, line, EXIT_FAILURE, "Function does not return a value");
        if (rhscall && rhscall->returnValuesToPop.size() > 1)
            errprintln(file + ":" + to_string(line),
                       "Warning: Function returns more than one value. Using only the first return value.");
        outputAutoMoveInstruction(file, line, state.symbolTable, op, out, rhs->out);
        state.symbolTable.pop();

        state.symbolTable.pop();
    } else if (op == OP_MOV) {
        // todo: optimize expressions, that override itself by preventing uneccesary copies of references to temporary variables
        if (lhstuple && rhstuple) {
            // move values from one tuple to another
            if (lhstuple->tuple.size() != rhstuple->tuple.size()) {
                die(file, line, EXIT_FAILURE,
                    "Can't evaluate unsimilar tuples (length " + to_string(lhstuple->tuple.size()) + " vs. " +
                    to_string(rhstuple->tuple.size()) + ")");
            }
            for (int i = 0; i < lhstuple->tuple.size(); i++) {
                auto lhs = lhstuple->tuple[i];
                lhs->compile(state);
                if (lhs->out.resolved->temp)
                    die(file, line, EXIT_FAILURE, "Can't assign to temporary", dereference(file, line, lhs->out));
                auto rhs = rhstuple->tuple[i];
                rhs->dst = lhs->out;
                state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
                rhs->compile(state);
                outputAutoMoveInstruction(file, line, state.symbolTable, op, lhs->out, rhs->out);
                state.symbolTable.pop();
            }
        } else if (lhstuple && rhscall) {
            rhscall->compile(state);

            // move return values of method call into tuple
            if (lhstuple->tuple.size() != rhscall->returnValuesToPop.size()) {
                die(file, line, EXIT_FAILURE,
                    "Can't return values (" + to_string(lhstuple->tuple.size()),
                    "destinations vs.", rhscall->returnValuesToPop.size(), "values to return)");
            }


            for (int i = 0; i < lhstuple->tuple.size(); i++) {
                auto lhs = lhstuple->tuple[i];
                lhs->compile(state);
                if (lhs->out.resolved->temp)
                     die(file, line, EXIT_FAILURE, "Can't assign to temporary", dereference(file, line, lhs->out));
                if (!rhs->out.resolved->temp)
                    die(file, line, EXIT_FAILURE, "Returned reference?", dereference(file, line, rhs->out));
                auto rhs = rhscall->returnValuesToPop[i];
                outputMoveInstruction(file, line, op, lhs->out, rhs);
            }
        } else if (lhscall) {
            die(file, line, EXIT_FAILURE, "Can't assign to function call");
        } else {
            lhs->compile(state);
            out = lhs->out;
            if (asVariable(file, line, lhs->out)->temp)
                die(file, line, EXIT_FAILURE, "Can't assign to temporary", dereference(file, line, lhs->out));
            rhs->dst = out;
            rhs->compile(state);
            if (rhscall && rhscall->returnValuesToPop.size() == 0)
                die(file, line, EXIT_FAILURE, "Function does not return a value");
            outputAutoMoveInstruction(file, line, state.symbolTable, op, out, rhs->out);
        }
    } else die(file, line, EXIT_FAILURE, "Unimplemented operator", binop2str(op));
}

BinaryOperatorExpression::~BinaryOperatorExpression() {
    delete lhs;
    lhs = nullptr;
    delete rhs;
    rhs = nullptr;
}

BinaryOperatorExpression::BinaryOperatorExpression(BinaryOperatorExpression::OperatorType op, Expression *lhs,
                                                   Expression *rhs)
        : op(op), lhs(lhs), rhs(rhs) {}

void DotExpression::compile(CompilationState &state) {
    lhs->out = out;
    lhs->compile(state);
    rhs->out = lhs->out;
    rhs->compile(state);
    out = rhs->out;
}

DotExpression::~DotExpression() {
    delete lhs;
    lhs = nullptr;
    delete rhs;
    rhs = nullptr;
}

DotExpression::DotExpression(Expression *lhs, Expression *rhs)
        : lhs(lhs), rhs(rhs), arg(nullptr) {}

void CallExpression::compile(CompilationState &state) {
    fun->compile(state);
    auto asfun = asFunction(file, line, fun->out);

    // Create accessable variables for the return values
    for (auto &retvar : asfun->returnValues)
        returnValuesToPop.push_back(state.symbolTable.newTmpVariable(line, asVariable(file, line, retvar)->type));
    // use the first return value as default output in expressions
    if (returnValuesToPop.size() > 0)
        out = returnValuesToPop.front();

    state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));

    // reserve space for the return variable
    state.symbolTable.initFunctionStackframe(file, line, true);

    auto calleeReturnCell = (int) state.symbolTable.findGlobal(QualifiedName{"__ret"}).dereference(file, line);

    // handle 'this' semantic, if function is a member of a type
    if (asfun->memberOf) {
        // find this object (-> last object in resolution path)
        auto thisObject = state.symbolTable.findGlobal(QualifiedName{fun->out.resolutionPath[0]->name});
        for (int i = 1; i < fun->out.resolutionPath.size() - 1; i++)
            thisObject.find(fun->out.resolutionPath[i]->name);
        // create the parameter variable for the 'this' object
        auto thisVariable = state.symbolTable.newTmpVariable(line, asVariable(file, line, thisObject)->type, -1, "__this");
        if (fun->out.resolutionPath.size() < 2)
            die(file, line, EXIT_FAILURE, "Member function not called by a member");
        // Create temporary variable and stackframe, needed for copying this object
        if (thisObject.resolved->temp) {
            outputMoveInstruction(file, line, BinaryOperatorExpression::OP_MOV, thisVariable, thisObject);
        } else {
            state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
            auto tempvar = state.symbolTable.newTmpVariable(line, state.cellType);
            outputCopyInstruction(file, line, BinaryOperatorExpression::OP_MOV, thisVariable, thisObject, tempvar);
            state.symbolTable.pop();
        }
    }

    // prepare the expressions for each argument, depending on if the argument list is a tuple or not
    std::vector<Expression*> argumentExprVec;
    auto astuple = dynamic_cast<TupleExpression*>(arguments);
    if (astuple != nullptr) argumentExprVec = astuple->tuple;
    else if (arguments != nullptr) argumentExprVec.push_back(arguments);

    if (argumentExprVec.size() != asfun->parameters.size() - (asfun->memberOf != nullptr ? 1 : 0))
        die(file, line, EXIT_FAILURE, "Expected", asfun->parameters.size() - (asfun->memberOf != nullptr ? 1 : 0), "arguments, but got", argumentExprVec.size());

    for (int i = 0; i < asfun->parameters.size() - (asfun->memberOf != nullptr ? 1 : 0); i++) {
        auto argvar = asfun->parameters[i + (asfun->memberOf != nullptr ? 1 : 0)];
        auto argexpr = argumentExprVec[i];
        argumentsToPush.push_back(state.symbolTable.newTmpVariable(line, asVariable(file, line, argvar)->type, -1, "__arg"));
        auto argframe = state.symbolTable.newTmpStackframe(line);
        state.symbolTable.push(*argframe);
        argexpr->compile(state);
        if (argexpr->out.resolved->temp) {
            // push argument expression on top of the stack
            outputMoveInstruction(file, line, BinaryOperatorExpression::OP_MOV, argumentsToPush.back(), argexpr->out);
        } else {
            // copy the variable before pushing it on the stack
            auto tempvar = state.symbolTable.newTmpVariable(line, state.cellType);
            outputCopyInstruction(file, line, BinaryOperatorExpression::OP_MOV, argumentsToPush.back(), argexpr->out,
                                  tempvar);
        }
        state.symbolTable.pop();
    }

    auto calleeArgumentsEnd = (int) state.symbolTable.currentScope()->getCurrentAddressOfFunctionStackframeEnd();

    state.symbolTable.pop();

    Instruction call(file, line, InstructionName::CALL, asfun->name);
    call.call.returnCell = calleeReturnCell;
    call.call.returnAddress = ++jumpAddressCounter;
    outputInstruction(call);

    outputJumpInstruction(file, line, calleeArgumentsEnd, asfun->address, asfun->name + "@" + to_string(asfun->address));
    // create the label, the callee will jump back to
    outputLabelInstruction(file, line, calleeReturnCell, call.call.returnAddress, "ret-" + asfun->name);
}

CallExpression::~CallExpression() {
    delete fun;
    fun = nullptr;
    delete arguments;
    arguments = nullptr;
}

CallExpression::CallExpression(Expression *fun, Expression *arguments)
        : fun(fun), arguments(arguments) {}

void ListStatement::compile(CompilationState &state) {
    bool pop_flag;
    if (function != nullptr) {
        if (function != state.symbolTable.currentScope()) {
            state.symbolTable.push(*function);
            pop_flag = true;
        } else {
            pop_flag = false;
        }
    } else {
        state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
        pop_flag = true;
    }

    for (auto stmt : *list)
        stmt->compile(state);
    // todo: if list is temporary, delete also all created symbols as they are not reachable
    if (pop_flag)
        state.symbolTable.pop();
}

ListStatement::~ListStatement() {
    if (list) {
        for (Statement *stmt : *list)
            delete stmt;
        delete list;
        list = nullptr;
    }
    function = nullptr;
}

ListStatement::ListStatement(vector<Statement *> *list) : function(nullptr), list(list) {}

void VariableDefinition::compile(CompilationState &state) {
    TypeSymbol *astype = state.cellType;

    if (type != nullptr) {
        auto result = state.symbolTable.findGlobal(*type->typeName);
        if (!result)
            die(file, line, EXIT_FAILURE, "Undefined type: " + joinQualified(*type->typeName));
        astype = dynamic_cast<TypeSymbol*>(result.resolved);

        if (astype == nullptr)
            die(file, line, EXIT_FAILURE, "Expected type, found " + symbol2str(*result.resolved) + " " + joinQualified(*type->typeName));
    }

    variableSymbol = new VariableSymbol(line, file, *name, astype);

    if (type != nullptr) {
        variableSymbol->length = type->length;
        variableSymbol->isPointerType = type->type != VariableType::STACK;
    }

    state.symbolTable.add(*variableSymbol);
}

VariableDefinition::~VariableDefinition() {
    // don't delete the symbol
    variableSymbol = nullptr;
    delete type;
    type = nullptr;
    delete name;
    name = nullptr;
}

VariableDefinition::VariableDefinition(VariableType *type, string *name)
        : type(type), name(name) {}

void TypeStatement::compile(CompilationState &state) {
    auto newtype = new TypeSymbol(line, file, *name);
    state.symbolTable.add(*newtype);
    state.symbolTable.push(*newtype);
    for (auto var : *variables)
        var->compile(state);
    state.symbolTable.pop();
}

TypeStatement::~TypeStatement() {
    delete name;
    name = nullptr;
    for (VariableDefinition *def : *variables)
        delete def;
    delete variables;
    variables = nullptr;
}

void FunctionStatement::compile(CompilationState &state) {
    auto functionSymbol = new FunctionSymbol(line, file, qualifiedName->back(), ++jumpAddressCounter);

    // register main function if applicable
    if (state.symbolTable.scopeStack.size() == 1 && qualifiedName->back() == "main") {
        if (state.main != nullptr)
            die(file, line, EXIT_FAILURE, "Main function already defined here:", *state.main);
        state.main = functionSymbol;
    }

    // push the extended scope if applicable
    if (qualifiedName->size() > 1) {
        QualifiedName extendedScope = *qualifiedName;
        extendedScope.pop_back();
        auto exScopeResult = state.symbolTable.findGlobal(extendedScope);
        if (!exScopeResult)
            die(file, line, EXIT_FAILURE, "Can't extend scope " + joinQualified(extendedScope) + " with function " + qualifiedName->back());
        if (dynamic_cast<const TypeSymbol*>(exScopeResult.resolved) == nullptr)
            die(file, line, EXIT_FAILURE, "Only types are extendable, but this extends", symbol2str(*exScopeResult.resolved));
        state.symbolTable.push(*exScopeResult.resolved);
        functionSymbol->memberOf = exScopeResult.resolved;
    }

    state.symbolTable.add(*functionSymbol);
    state.symbolTable.push(*functionSymbol);

    // create the variables for the return values
    if (returnVariables != nullptr) {
        for (auto var : *returnVariables) {
            var->compile(state);
            functionSymbol->returnValues.push_back(state.symbolTable.findGlobal(QualifiedName{var->variableSymbol->name}));
        }
    }

    // create the function stack base variables
    state.symbolTable.initFunctionStackframe(file, line, false);

    // create all parameters
    if (parameterVariables != nullptr) {
        for (auto var : *parameterVariables) {
            var->compile(state);
            functionSymbol->parameters.push_back(state.symbolTable.findGlobal(QualifiedName{var->variableSymbol->name}));
        }
    }


    auto endOfStack = static_cast<int>(state.symbolTable.currentScope()->getCurrentAddressOfFunctionStackframeEnd());

    // create the function label
    outputLabelInstruction(file, line, endOfStack, functionSymbol->address, joinQualified(functionSymbol->getQualified()));

    // prevent the list from creating a temporary stackframe by registering the parent function
    auto *functionBodyList = dynamic_cast<ListStatement*>(functionBody);
    if (functionBodyList != nullptr)
        functionBodyList->function = functionSymbol;

    // compile the function
    functionBody->compile(state);

    auto returnRegisterAddress = (int) state.symbolTable.findGlobal(QualifiedName{"__ret"}).dereference(file, line);
    // pop the function scope
    state.symbolTable.pop();

    // pop the extended type's scope
    if (functionSymbol->memberOf != nullptr)
        state.symbolTable.pop();

    Instruction ret(file, line, InstructionName::RET, functionSymbol->name);
    ret.ret.ret = returnRegisterAddress;
    ret.ret.exit = functionSymbol->name == "main";
    outputInstruction(ret);
}

FunctionStatement::~FunctionStatement() {
    delete qualifiedName;
    if (parameterVariables) {
        for (VariableDefinition *def : *parameterVariables)
            delete def;
        delete parameterVariables;
    }
    if (returnVariables) {
        for (ReturnVariableDefinition *def : *returnVariables)
            delete def;
        delete returnVariables;
    }
    delete functionBody;

    qualifiedName = nullptr;
    parameterVariables = nullptr;
    returnVariables = nullptr;
    functionBody = nullptr;

}

FunctionStatement::FunctionStatement(QualifiedName *qualifiedName, vector<VariableDefinition *> *parameterVariables,
                                     vector<ReturnVariableDefinition *> *returnVariables, Statement *functionBody)
        :   qualifiedName(qualifiedName),
            parameterVariables(parameterVariables),
            returnVariables(returnVariables),
            functionBody(functionBody) {}

size_t TypeSymbol::getSizeSumOfChildSymbols() const {
    size_t out = 0;
    for (auto s : symbols)
        out += s->getSizeOnTheStack();
    return out;
}

size_t VariableSymbol::getSizeSumOfChildSymbols() const {
    return type->getSizeSumOfChildSymbols() * length;
}

size_t VariableSymbol::getSizeOnTheStack() const {
    return getSizeSumOfChildSymbols();
}

size_t VariableSymbol::getAddressRelativeToParent() const {
    size_t loc = 0;
    for (auto s : parent->symbols) {
        if (s == this)
            return loc;
        loc += s->getSizeOnTheStack();
    }
    die(file, line, EXIT_FAILURE, "No location for variable found");
    return 0;
}

size_t VariableSymbol::getAddressRelativeToFunctionStackframe() const {
    return parent->getAddressRelativeToFunctionStackframe() + getAddressRelativeToParent();
}

VariableSymbol::VariableSymbol(int line, string file, string name, TypeSymbol *type)
        : Symbol(line, file, name), type(type) {}

std::string VariableSymbol::type2str() const {
    if (verboseSymbolNames) {
        if (isPointerType) {
            return joinQualified(type->getQualified()) + "*" + to_string(length);
        } else {
            return joinQualified(type->getQualified());
        }
    }
    if (isPointerType)
        return type->name + "*" + to_string(length);
    return type->name;
}

size_t StackframeSymbol::getSizeSumOfChildSymbols() const {
    size_t size = 0;
    for (auto s : symbols)
        size += s->getSizeOnTheStack();
    return size;
}

size_t StackframeSymbol::getAddressRelativeToParent() const {
    size_t loc = 0;
    for (auto s : parent->symbols)
        loc += s->getSizeOnTheStack();
    return loc;
}

size_t StackframeSymbol::getAddressRelativeToFunctionStackframe() const {
    if (parent == nullptr)
        return 0;
    return parent->getAddressRelativeToFunctionStackframe() + getAddressRelativeToParent();
}

StackframeSymbol::StackframeSymbol(int line, string file, std::string name) : Symbol(line, file, name) {}

size_t SymbolResolutionResult::dereference(const string &file, int line) const {
    size_t adr = 0;
    for (auto s : resolutionPath) {
        if ((dynamic_cast<const FunctionSymbol*>(s) != nullptr) || dynamic_cast<const TypeSymbol*>(s))
            die(file, line, EXIT_FAILURE, "Can't dereference", *s);
        auto asvar = dynamic_cast<const VariableSymbol*>(s);
        if (asvar != nullptr)
            adr += asvar->getAddressRelativeToFunctionStackframe();
    }
    return adr;
}

SymbolResolutionResult &SymbolResolutionResult::find(string name) {
    resolved = nullptr;
    for (auto s : scope->getScope()->symbols) {
        if (s->name == name) {
            scope = s;
            resolved = s;
            resolutionPath.push_back(s);
        }
    }
    return *this;
}

QualifiedName SymbolResolutionResult::qualified() const {
    QualifiedName qualified;
    qualified.reserve(resolutionPath.size());
    for (auto s : resolutionPath)
        qualified.push_back(s->name);
    return qualified;
}

string SymbolResolutionResult::getTrace() const {
    stringstream ss;
    for (auto s : resolutionPath)
        ss << "At: " << *s << endl;
    return ss.str();
}

std::string SymbolResolutionResult::name() const {
    if (verboseSymbolNames)
        return joinQualified(qualified());
    return resolved->name;
}

void IfStatement::compile(CompilationState &state) {
    auto trueLabel = ++jumpAddressCounter;
    auto falseLabel = onFalse != nullptr ? ++jumpAddressCounter : -1;
    auto fiLabel = ++jumpAddressCounter;

    state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
    condition->compile(state);
    state.symbolTable.pop();
    auto jumpRegister = (int) state.symbolTable.currentScope()->getCurrentAddressOfFunctionStackframeEnd();

    if (!condition->out)
        die(file, line, EXIT_FAILURE, "Invalid conditional");

    if (onFalse != nullptr)
        outputTestInstruction(file, line, condition->out, jumpRegister, trueLabel, falseLabel);
    else
        outputTestInstruction(file, line, condition->out, jumpRegister, trueLabel, fiLabel);

    outputLabelInstruction(file, line, jumpRegister, trueLabel, "IF_TRUE");
    state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
    onTrue->compile(state);
    state.symbolTable.pop();
    outputJumpInstruction(file, line, jumpRegister, fiLabel, "FI");

    if (onFalse != nullptr) {
        outputLabelInstruction(file, line, jumpRegister, falseLabel, "IF_FALSE");
        state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
        onFalse->compile(state);
        state.symbolTable.pop();
        outputJumpInstruction(file, line, jumpRegister, fiLabel, "FI");
    }

    outputLabelInstruction(file, line, jumpRegister, fiLabel, "FI");
}

IfStatement::~IfStatement() {
    delete condition;
    condition = nullptr;
    delete onTrue;
    onTrue = nullptr;
    delete onFalse;
    onFalse = nullptr;
}

IfStatement::IfStatement(Expression *condition, Statement *onTrue, Statement *onFalse)
        : condition(condition), onTrue(onTrue), onFalse(onFalse) {}


void WhileStatement::compile(CompilationState &state) {
    // label for where the condition is evaluated
    auto conditionLabel = ++jumpAddressCounter;
    // label for where the body is evaluated
    auto trueLabel = ++jumpAddressCounter;
    // label for after the condition is false
    auto falseLabel = ++jumpAddressCounter;
    // address for registers required to for jump
    auto jump_register = (int) state.symbolTable.currentScope()->getCurrentAddressOfFunctionStackframeEnd();

    // jump to the condition for the first time
    outputJumpInstruction(file, line, jump_register, conditionLabel, "WHILE");

    // create label for the condition
    outputLabelInstruction(file, line, jump_register, conditionLabel, "WHILE_CONDITION");

    // evaluate the condition
    state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
    condition->compile(state);
    state.symbolTable.pop();

    // jump according to the conditions result
    outputTestInstruction(file, line, condition->out, jump_register, trueLabel, falseLabel);
    // create label for the body
    outputLabelInstruction(file, line, jump_register, trueLabel, "WHILE_BODY");

    // evaluate the body
    state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
    body->compile(state);
    state.symbolTable.pop();

    // jump back to evaluate the condition
    outputJumpInstruction(file, line, jump_register, conditionLabel, "WHILE_CONDITION");
    // label for when the condition is false
    outputLabelInstruction(file, line, jump_register, falseLabel, "WHILE_FALSE");
}

WhileStatement::~WhileStatement() {
    delete condition;
    condition = nullptr;
    delete body;
    body = nullptr;
}

WhileStatement::WhileStatement(Expression *condition, Statement *body)
        : condition(condition), body(body) {}

TupleExpression::TupleExpression(Expression *lhs, Expression *rhs) {
    TupleExpression *lhscast, *rhscast;
    lhscast = dynamic_cast<TupleExpression *>(lhs);
    rhscast = dynamic_cast<TupleExpression *>(rhs);

    assert(!lhscast || !rhscast);

    if (lhscast != nullptr) {
        for (auto ptr : lhscast->tuple)
            tuple.push_back(ptr);
        lhscast->tuple.clear();
        delete lhs;
    } else
        tuple.push_back(lhs);

    if (rhscast != nullptr) {
        for (auto ptr : rhscast->tuple)
            tuple.push_back(ptr);
        rhscast->tuple.clear();
        delete rhs;
    } else
        tuple.push_back(rhs);
}

void TupleExpression::compile(CompilationState &state) {
    errprintln(file + ":" + to_string(line), "TUPLE");
    assert(false);
}

TupleExpression::~TupleExpression() {
    for (Expression *expr : tuple)
        delete expr;
}

void ReturnStatement::compile(CompilationState &state) {
    auto fun = state.symbolTable.currentScope()->getParentFunctionStackframe();
    if (fun == nullptr)
        die(file, line, EXIT_FAILURE, "Return outside of function");
    if (expr != nullptr) {
        auto astuple = dynamic_cast<TupleExpression*>(expr);

        // return action if returning multiple values
        if (astuple != nullptr) {
            if (astuple->tuple.size() != fun->returnValues.size())
                die(file, line, EXIT_FAILURE, "Too", astuple->tuple.size() < fun->returnValues.size()  ? "few" : "many", "values to return", "(" + to_string(astuple->tuple.size()), "vs.", to_string(fun->returnValues.size()) + ")");
            int i = 0;
            for (auto e : astuple->tuple) {
                // pair each return expression with it's corresponding return variable
                auto tmp = state.symbolTable.newTmpStackframe(line);
                state.symbolTable.push(*tmp);
                e->compile(state);
                if (!e->out)
                    die(file, line, EXIT_FAILURE, "Undefined return value");
                if (e->out.resolved->temp) {
                    outputMoveInstruction(file, line, BinaryOperatorExpression::OP_MOV, fun->returnValues[i], e->out);
                } else {
                    auto tmpvar = state.symbolTable.newTmpVariable(line, state.cellType);
                    outputCopyInstruction(file, line, BinaryOperatorExpression::OP_MOV, fun->returnValues[i], e->out,
                                          tmpvar);
                }
                i += 1;
                state.symbolTable.pop();
            }
        } else {
            // return action if returning single value
            if (fun->returnValues.empty())
                die(file, line, EXIT_FAILURE, "The function does not return a value");
//            else if (fun->returnValues.size() > 1)
//                die(file, line, EXIT_FAILURE, "Too many values to return", "(1 vs.", to_string(fun->returnValues.size()) + ")");
            auto tmp = state.symbolTable.newTmpStackframe(line);
            state.symbolTable.push(*tmp);
            expr->compile(state);
            if (!expr->out)
                die(file, line, EXIT_FAILURE, "Undefined return value");
            if (expr->out.resolved->temp) {
                outputMoveInstruction(file, line, BinaryOperatorExpression::OP_MOV, fun->returnValues[0], expr->out);
            } else {
                auto tmpvar = state.symbolTable.newTmpVariable(line, state.cellType);
                outputCopyInstruction(file, line, BinaryOperatorExpression::OP_MOV, fun->returnValues[0], expr->out,
                                      tmpvar);
            }
            state.symbolTable.pop();
        }
    } else {
        // todo: return nothing for void functions
    }
    // todo: leave function
}

ReturnStatement::~ReturnStatement() {
    delete expr;
    expr = nullptr;
}

void IOStatement::compile(CompilationState &state) {
    auto astuple = dynamic_cast<TupleExpression*>(expr);
    state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
    if (astuple != nullptr) {
        for (auto e : astuple->tuple) {
            state.symbolTable.push(*state.symbolTable.newTmpStackframe(line));
            e->compile(state);
            if (!e->out)
                die(file, line, EXIT_FAILURE, "No destination found");
            if (e->out.resolved->temp && function == IOFunction::IOINPUT)
                die(file, line, EXIT_FAILURE, "Input destination can't be a temporary");
            outputIoInstruction(file, line, function, e->out);
            state.symbolTable.pop();
        }
    } else {
        expr->compile(state);
        if (!expr->out)
            die(file, line, EXIT_FAILURE, "No destination found");
        if (expr->out.resolved->temp && function == IOFunction::IOINPUT)
            die(file, line, EXIT_FAILURE, "Input destination can't be a temporary");
        outputIoInstruction(file, line, function, expr->out);
    }
    state.symbolTable.pop();
}

void InlineStatement::compile(CompilationState &state) {
    Instruction i(file, line, InstructionName::WRITE_INLINE, "inline");
    i.inline_code.inlineStr = inl->c_str();
    outputInstruction(i);
}

InlineStatement::~InlineStatement() {
    delete inl;
    inl = nullptr;
}

std::ostream &operator<<(std::ostream &os, const Instruction &i) {
    switch (i.instr) {
        case InstructionName::NOP:
            break;
        case InstructionName::COPY:
        case InstructionName::ADD_COPY:
        case InstructionName::SUB_COPY:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug)
                    os << endl;
                if (i.instr == InstructionName::COPY)
                    zero(os, i.copy.dst, i.copy.size);
                if (i.instr == InstructionName::SUB_COPY)
                    sub(os, i.copy.dst, i.copy.src, i.copy.aux, i.copy.size);
                else
                    add(os, i.copy.dst, i.copy.src, i.copy.aux, i.copy.size);
            }
            return os;
        case InstructionName::ILOAD:
        case InstructionName::IADD:
        case InstructionName::ISUB:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                if (i.instr == InstructionName::ILOAD)
                    zero(os, i.constant.dst, i.constant.size);
                iadd(os, i.constant.dst, i.constant.value, i.constant.size);
            }
            return os;
        case InstructionName::MOVE:
        case InstructionName::ADD:
        case InstructionName::SUB:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                if (i.instr == InstructionName::MOVE)
                    zero(os, i.move.dst, i.move.size);
                if (i.instr == InstructionName::SUB)
                    sub(os, i.move.dst, i.move.src, i.move.size);
                else
                    add(os, i.move.dst, i.move.src, i.move.size);
            }
            return os;
        case InstructionName::COMPARE:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                zero(os, i.compare.isZero, 1);
                inc(os, i.compare.isZero);
                zero(os, i.compare.notZero, 1);
                insertAt(os, i.compare.conditionAddress, "[[-]");
                inc(os, i.compare.notZero);
                dec(os, i.compare.isZero);
                insertAt(os, i.compare.conditionAddress, "]");
            }
            return os;
        case InstructionName::PUSH_STACK:
        case InstructionName::POP_STACK:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.stack.offset, i.comment);
            if (i.release) {
                if (debug) os << endl;
                if (i.instr == InstructionName::POP_STACK)
                    movePtr(os, -i.stack.offset);
                else
                    movePtr(os, i.stack.offset);
            }
            return os;
        case InstructionName::WRITE_INPUT:
        case InstructionName::WRITE_OUTPUT:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                foreach(os, i.io.src, i.io.size, i.instr == InstructionName::WRITE_INPUT ? "," : ".");
            }
            return os;
        case InstructionName::TEST:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                zero(os, i.test.jumpRegister, 1);

                insertAt(os, i.test.isTrue, "[[-]");
                zero(os, i.test.jumpRegister, 1);
                iadd(os, i.test.jumpRegister, i.test.trueLabel, 1);
                insertAt(os, i.test.isTrue, "]");

                insertAt(os, i.test.isFalse, "[[-]");
                zero(os, i.test.jumpRegister, 1);
                iadd(os, i.test.jumpRegister, i.test.falseLabel, 1);
                insertAt(os, i.test.isFalse, "]");
                osprint(os<<endl, string(static_cast<unsigned long>(i.test.jumpRegister), '>'));
                osprint(os, ">[-]>[-]+<<>]<>]>[[-]<+>]<");
            }
            return os;
        case InstructionName::CALL:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                zero(os, i.call.returnCell, 1);
                iadd(os, i.call.returnCell, i.call.returnAddress, 1);
            }
            return os;
        case InstructionName::RET:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                // todo: replace ">[-]>[-]+<<[-]" with ">[-]>-*self_address+<<[-]" maybe
                osprint(os, string(static_cast<unsigned long>(i.ret.ret), '>') + ">[-]>[-]" + (i.ret.exit ? "":"+") + "<<>]<>]>[[-]<+>]<");
            }
            return os;
        case InstructionName::LABEL:
            if (!i.release || debug)
                osprint(os << endl, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                osprint(os, "[[-]>[-]<<[->+>+<<]>[-<+>]+<>>" + string(i.label.address, '-') + "[[-]<->]<<>>+<<>[<");
                /**
                    +   main function is target
                    >+  marker at target+1
                    [   enter program

                    [[-]>[-]<<                          clear target+1 and target + 2
                    [->+>+<<]>[-<+>]+<                  copy target to target+2 and set target+1 to 1
                    >>fun_adr[[-]<->]<<                 subtract own address from target+2 and set target+1 to 0 if target+2 is not equal to fun_adr
                    >>+<<                               set target+2 to 1
                    >[<                                 if target+1 is 1, execute custom code
                                                        custom code; finishes on new 'target' cell with desired new address
                    >[-]>[-]continue(1|0)<<[-]target    clear target+1; set target+2 to 1 for continue or 0 to exit; set next jump at target to desired address
                    >]<
                    >]                                  exit at target+1

                    >[[-]<+>]<                          set target+1 to 1 if target+2 is not 0;

                    ]                                   exit program
                 */
            }
            return os;
        case InstructionName::WRITE_INLINE:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.inline_code.inlineStr);
            if (i.release)
                osprint(os, i.inline_code.inlineStr);
            return os;
        case InstructionName::JUMP:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release) {
                if (debug) os << endl;
                // todo: replace ">[-]>[-]+<<[-]" with ">[-]>-*self_address+<<[-]" maybe
                osprint(os, ">[-]>[-]+<<[-]" + string(i.jump.targetAddress, '+') + ">]<>]>[[-]<+>]<");
            }
            return os;
        case InstructionName::EXIT:
            if (!i.release || debug)
                osprint(os, i.line, instruction_name_map.at(i.instr), i.comment);
            if (i.release)
                osprint(os, "[-]" + string((unsigned long) i.exit.exitCode, '+') + "@");
            // todo: exit keyword
            return os;
        case InstructionName::UNINITIALIZED:
            die(i.file, i.line, EXIT_FAILURE, "Uninitialized instruction");
    }
    return os;
}

CompilationState::CompilationState() : main(nullptr) {
    // Initialize SymbolTable with the native type of "cell"
    struct CellSymbol : TypeSymbol {
        size_t getSizeSumOfChildSymbols() const override { return 1; }
        CellSymbol() : TypeSymbol(0, "<init>", "cell") {}
    };
    cellType = new CellSymbol();
    symbolTable.add(*cellType);
}

void ExpressionStatement::compile(CompilationState &state) {
    auto frame = state.symbolTable.newTmpStackframe(line);
    state.symbolTable.push(*frame);
    expr->compile(state);
    state.symbolTable.pop();
}

VariableStatement::~VariableStatement() {
    for (VariableDefinition *def : *variables)
        delete def;
    delete variables;
    variables = nullptr;
}

void VariableStatement::compile(CompilationState &state) {
    for (auto var : *variables)
        var->compile(state);
}

VariableStatement::VariableStatement(vector<VariableDefinition *> *variables) : variables(variables) { }

VariableType::~VariableType() {
    delete typeName;
    typeName = nullptr;
}

VariableType::VariableType(VariableType::TypeMemoryTag type, int length, QualifiedName *typeName)
        : type(type), length(length), typeName(typeName) {}

Expression::Expression() : out(nullptr), dst(nullptr) {}

Expression::~Expression() {}

ASTNode::ASTNode() : line(yylineno), column(columnno), len(yyleng), file(currentFile) {}

ASTNode::~ASTNode() {}

size_t FunctionSymbol::getSizeSumOfChildSymbols() const {
    size_t size = 0;
    for (auto s : symbols)
        size += s->getSizeOnTheStack();
    return size;
}

FunctionSymbol::FunctionSymbol(int line, string file, string name, int address)
        : Symbol(line, file, name), address(address) {}

StringExpression::StringExpression(std::string *str) : string(str) {}

StringExpression::~StringExpression() {
    delete string;
    string = nullptr;
}

void StringExpression::compile(CompilationState &state) {
    std::string parsed = parseStringEscape(*string);
    out = state.symbolTable.newTmpVariable(line, state.cellType, static_cast<int>(parsed.size()));
    outputLoadStringInstruction(file, line, out, parsed);
}
