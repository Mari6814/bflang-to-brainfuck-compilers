#ifndef BF_H
#define BF_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <map>
#include <array>
#include "print.h"

struct SymbolTable;
struct Symbol;
struct Statement;
struct TypeSymbol;
struct FunctionSymbol;
struct VariableSymbol;
struct VariableDefinition;
struct StackframeSymbol;
struct ReturnVariableDefinition;
struct VariableType;
struct FunctionStatement;
struct ListStatement;

// Path needed to qualify a symbol reference
typedef std::vector<std::string> QualifiedName;
// Relative reference to a cell
typedef int cellReference;
// Size of the object pointet to by a cell reference
typedef int cellSize;
// An address, that is executable
typedef int label;
// Constant values like offset or integers
typedef int cellValue;

extern FILE *yyin;
extern int columnno;
extern int yylineno;
extern size_t yyleng;
extern int yylex(void);
extern void yyerror(const char *s);
extern int yyparse();
extern ListStatement *bisonAST;
extern void yyrestart(FILE*);
extern std::string currentFile;
extern std::ofstream binary_output_stream;
extern std::ofstream intermediate_output_stream;
extern bool verbose;
extern bool verboseSymbolTable;
extern bool debug;
extern bool verboseSymbolNames;

using namespace std;

ostream &operator<<(ostream &os, const Symbol &symbol);
ostream &operator<<(ostream &os, const SymbolTable &st);

#define EXIT_ON_DIE

struct Location {
    // todo: use this instead of only the line number
    int line;
    int column;
    string file;
};

enum class InstructionName {
    // throws error
    UNINITIALIZED,
    // will be ignored
    NOP,
    // sets all 'size' bytes at 'dst' to 'constant'
    ILOAD,
    // moves 'size' bytes from src to dst
    MOVE,
    COMPARE,
    // copies 'size' bytes from src to dst, using 'size_aux' auxilliary bytes at aux
    COPY,
    // adds 'size' bytes from src to dst
    ADD,
    // adds 'const' to all 'size' bytes at 'dst'
    IADD,
    // adds 'size' bytes from src to dst, using 'size_aux' auxilliary bytes at aux
    ADD_COPY,
    // subs 'size' bytes from src to dst
    SUB,
    // subs 'const' to all 'size' bytes at 'dst'
    ISUB,
    // subs 'size' bytes from src to dst, using 'size_aux' auxilliary bytes at aux
    SUB_COPY,
    // moves the pointer to the current top of the stack at offset 'offset'
    PUSH_STACK,
    // moves the pointer 'offset' bytes from the top of the stack
    POP_STACK,
    // writes the next 'size' inputs into 'dst' with size 'size'
    WRITE_INPUT,
    // outputs the 'size' bytes from src
    WRITE_OUTPUT,
    // if 'condition' is 0, write the value of 'true' in the jump register, and the value of 'false' if it is not
    TEST,
    // writes the address of the target 'jump_target' into the jump register and the address of the return point into 'return_target'
    CALL,
    // signals end of function
    RET,
    // writes the address 'jump_target' into the jump register without setting a return point
    JUMP,
    // writes inline bf code
    WRITE_INLINE,
    // label marker
    LABEL,
    // exits the program with the value at the pointer as exit code
    EXIT
};

const std::map<InstructionName, const char*> instruction_name_map {
        {InstructionName::NOP, "NOP"},
        {InstructionName::ILOAD, "ILOAD"},
        {InstructionName::MOVE, "MOVE"},
        {InstructionName::COMPARE, "COMPARE"},
        {InstructionName::COPY, "COPY"},
        {InstructionName::ADD, "ADD"},
        {InstructionName::IADD, "IADD"},
        {InstructionName::ADD_COPY, "ADD_COPY"},
        {InstructionName::SUB, "SUB"},
        {InstructionName::ISUB, "ISUB"},
        {InstructionName::SUB_COPY, "SUB_COPY"},
        {InstructionName::PUSH_STACK, "PUSH_STACK"},
        {InstructionName::POP_STACK, "POP_STACK"},
        {InstructionName::WRITE_INPUT, "INPUT"},
        {InstructionName::WRITE_OUTPUT, "OUTPUT"},
        {InstructionName::TEST, "TEST"},
        {InstructionName::CALL, "CALL"},
        {InstructionName::RET, "RETURN"},
        {InstructionName::JUMP, "JUMP"},
        {InstructionName::WRITE_INLINE, "INLINE"},
        {InstructionName::LABEL, ".L"},
        {InstructionName::EXIT, "EXIT"}
};

struct Instruction {
    std::string file;
    int line;
    InstructionName instr;
    bool release;
    union {
        struct {
            cellReference dst, src, aux;
            cellSize size, size_aux;
        } copy;

        struct {
            cellReference dst, src;
            cellSize size;
        } move;

        struct {
            cellReference conditionAddress;
            label isZero, notZero;
        } compare;

        struct {
            cellReference dst;
            cellSize size;
            cellValue value;
        } constant;

        struct {
            // cell to output
            cellReference src;
            // size of the cell
            cellSize size;
        } io;

        struct {
            // offset of the new stack
            cellValue offset;
        } stack;

        struct {
            // cell to where the adderss will be saved
            cellReference returnCell;
            // address to return to
            cellValue returnAddress;
        } call;

        struct {
            cellReference isTrue, isFalse, jumpRegister;
            label trueLabel, falseLabel;
        } test;

        struct {
            label targetAddress;
        } jump;

        struct {
            // Location of the cell, where the return address is stored (after the return values)
            cellReference ret;
            bool exit;
        } ret;

        struct {
            // String to be inlined
            const char *inlineStr;
        } inline_code;

        struct {
            // Jumpable address to the label
            label address;
        } label;

        struct {
            // Value to set the pointer to before exiting
            cellValue exitCode;
        } exit;
    };
    std::string comment;

    Instruction(const std::string file, int line, InstructionName instr = InstructionName::UNINITIALIZED, std::string comment = "")
            : file(file), line(line), instr(instr), comment(comment), release(false) {}
    friend std::ostream &operator<<(std::ostream &os, const Instruction &i);
};

struct Symbol {
    bool temp = false;
    // todo: use not accessed but allocated temporaries if possible
    bool used = false;
	// todo: inaccessable temporary/hidden symbols
	bool hidden = false;

    int line;
	string file, name;
	Symbol *parent;
	vector<Symbol*> symbols;

	Symbol(int line, string file, string name)
		: line(line), file(file), name(name), parent(nullptr) {};

	virtual ~Symbol() {
        for (Symbol *s : symbols)
            delete s;
        parent = nullptr;
    };

    // gets the concatenated symbol names up until the root
    QualifiedName getQualified() const;

    // Set parent member and adds this symbol as a child to the parent
    // returns nullptr if a parent was already set, else returns this
    void setParent(Symbol *newParent);

    // place that contains inner symbols
	virtual Symbol *getScope() { return this; };

    // gets the path from the root to this symbol
    vector<const Symbol*> fullPath() const;

    // todo: virtual really needed?
    virtual size_t getSizeSumOfChildSymbols() const { return 0; };
    virtual size_t getSizeOnTheStack() const { return 0; };
    virtual size_t getAddressRelativeToParent() const { return 0; };
    virtual size_t getAddressRelativeToFunctionStackframe() const { return 0; };
    // gives the address after this symbol
    size_t getCurrentAddressOfFunctionStackframeEnd() const { return getSizeSumOfChildSymbols() + getAddressRelativeToFunctionStackframe(); }
    // returns the next function in the hierarchy
    const FunctionSymbol *getParentFunctionStackframe() const;
};

struct SymbolResolutionResult {
    Symbol *resolved;
    Symbol *scope;
    vector<const Symbol*> resolutionPath;

    explicit SymbolResolutionResult(Symbol *scope)
            : resolved(nullptr), scope(scope) {}

    size_t dereference(const string &file, int line) const;

    SymbolResolutionResult &find(string name);

    QualifiedName qualified() const;

    string getTrace() const;

    std::string name() const;

    operator bool() const { return resolved != nullptr; }
    bool operator==(const SymbolResolutionResult &that) const { return *this && that && resolved == that.resolved; }
    bool operator!=(const SymbolResolutionResult &that) const { return !(*this == that); }
};

struct SymbolTable {
	vector<Symbol*> scopeStack;

	SymbolTable();
    ~SymbolTable() {
        assert(scopeStack.size() == 1);
        delete scopeStack[0];
    }

	// gets the top of the scope stack
    Symbol *currentScope() const;

	// pushes new current scope
	void push(Symbol &symbol);

	// adds a symbol to the current scope symbol
	void add(Symbol &symbol, bool temporary = false);

	// reverses push
	void pop();

    // todo: implement symbol removal for temporaries maybe?
    void remove(Symbol &symbol);

    // creates a stackframe for a function call
    void initFunctionStackframe(const std::string &file, int line, bool temporary);

    // searches a symbol in the current scope and it's parents
	SymbolResolutionResult findGlobal(QualifiedName qualified);

    // todo: table of jumpable locations / location manager

    // todo: offsets in variables (for arrays and input/output)

    SymbolResolutionResult newTmpVariable(int line, TypeSymbol *type, int length = -1, const char *debug_prefix = "__tmp");
    StackframeSymbol *newTmpStackframe(int line);
};

struct CompilationState {
    SymbolTable symbolTable;
    TypeSymbol *cellType;
    FunctionSymbol *main;
    CompilationState();
};

struct TypeSymbol : Symbol {
    TypeSymbol(int line, string file, string name) : Symbol(line, file, name) {}

    // todo: constructors by default value?
    size_t getSizeSumOfChildSymbols() const override;
};

struct VariableSymbol : Symbol {
    TypeSymbol *type;
	int length = 1;
	bool isPointerType = false;

    VariableSymbol(int line, string file, string name, TypeSymbol *type);

    size_t getSizeSumOfChildSymbols() const override;

    size_t getSizeOnTheStack() const override;

    size_t getAddressRelativeToParent() const override;

    size_t getAddressRelativeToFunctionStackframe() const override;

    Symbol *getScope() override { return type; }

	std::string type2str() const;
};

struct FunctionSymbol : Symbol {
	int address;
    Symbol *memberOf = nullptr;
    vector<SymbolResolutionResult> parameters;
    vector<SymbolResolutionResult> returnValues;

    FunctionSymbol(int line, string file, string name, int address);

    size_t getSizeSumOfChildSymbols() const override;
};

struct StackframeSymbol : Symbol {

    size_t getSizeSumOfChildSymbols() const override;

    size_t getAddressRelativeToParent() const override;

    size_t getAddressRelativeToFunctionStackframe() const override;

    StackframeSymbol(int line, string file, std::string name);
};

struct ASTNode {
	int line;
    int column;
    size_t len;
    string file;
	ASTNode();
	virtual ~ASTNode();
    virtual void compile(CompilationState &state) {}
};

struct Expression : ASTNode {
    SymbolResolutionResult out;
    // todo: use 'dst' expression output lookahead
    // todo: make 'dst' the expression, that calls "compile"?
    SymbolResolutionResult dst;

    Expression();
    ~Expression() override;
};

struct IntExpression : Expression {
	int integer;
    IntExpression(int integer) : integer(integer) {}

    void compile(CompilationState &state) override;
};

struct IdentifierExpression : Expression {
	string *identifier;

	IdentifierExpression(string *identifier);

    ~IdentifierExpression() override;

    void compile(CompilationState &state) override;
};

struct BinaryOperatorExpression : Expression {
    enum OperatorType {
        OP_MOV, OP_ADD, OP_SUB, OP_MUL, OP_DIV
    } op;
	Expression *lhs, *rhs;
	BinaryOperatorExpression(OperatorType op, Expression *lhs, Expression *rhs);

    ~BinaryOperatorExpression() override;

    void compile(CompilationState &state) override;
};

struct DotExpression : Expression {
    Expression *lhs, *rhs;

    SymbolResolutionResult arg;

    DotExpression(Expression *lhs, Expression *rhs);

    ~DotExpression() override;

    void compile(CompilationState &state) override;
};

struct CallExpression : Expression {
	Expression *fun;
	Expression *arguments;
    vector<SymbolResolutionResult> argumentsToPush;
    vector<SymbolResolutionResult> returnValuesToPop;

	CallExpression(Expression *fun, Expression *arguments);

    ~CallExpression() override;

    void compile(CompilationState &state) override;
};

struct VariableType : ASTNode {
    enum TypeMemoryTag {
        POINTER, FIXED, STACK
    } type;
	int length;
	QualifiedName *typeName;

	VariableType(TypeMemoryTag type, int length, QualifiedName *typeName);

    ~VariableType() override;
};

struct VariableDefinition : ASTNode {
	VariableType *type;
	string *name;

    VariableSymbol *variableSymbol;

	VariableDefinition(VariableType *type, string *name);

    virtual ~VariableDefinition() override;

    // todo: add assignment at initialization to variables
    virtual void compile(CompilationState &state) override;
};

struct ReturnVariableDefinition : VariableDefinition {
    ReturnVariableDefinition(VariableType *type, string *name) : VariableDefinition(type, name) {}
};

struct Statement : ASTNode {
	virtual ~Statement() {}
    virtual void compile(CompilationState &state) override {}
};

struct IfStatement : Statement {
	Expression *condition;
	Statement *onTrue, *onFalse;
	IfStatement(Expression *condition, Statement *onTrue, Statement *onFalse);
    // todo: nojump keyword for if statements, that don't jump

    ~IfStatement() override;

    void compile(CompilationState &state) override;
};

struct WhileStatement : Statement {
	Expression *condition;
	Statement *body;
    // todo: nojump keyword for while statements, that don't jump

	WhileStatement(Expression *condition, Statement *body);

    ~WhileStatement() override;

    void compile(CompilationState &state) override;
};

struct VariableStatement : Statement {
	vector<VariableDefinition*> *variables;

	VariableStatement(vector<VariableDefinition*> *variables);

    void compile(CompilationState &state) override;

    ~VariableStatement() override;

    // todo: implement initialization at definition in generate bytecode
};

struct TypeStatement : Statement {
    string *name;
    vector<VariableDefinition*> *variables;

	TypeStatement(string *name, vector<VariableDefinition*> *variables)
		: name(name), variables(variables) {}

    ~TypeStatement() override;

	void compile(CompilationState &state) override;
};

struct FunctionStatement : Statement {
	QualifiedName *qualifiedName;
    vector<VariableDefinition*> *parameterVariables;
	vector<ReturnVariableDefinition*> *returnVariables;
	Statement *functionBody;
	FunctionStatement(
					QualifiedName *qualifiedName,
					vector<VariableDefinition*> *parameterVariables,
					vector<ReturnVariableDefinition*> *returnVariables,
					Statement *functionBody);

    ~FunctionStatement();
	void compile(CompilationState &state) override;
};

struct ReturnStatement : Statement {
	Expression *expr;

    explicit ReturnStatement(Expression *expr) : expr(expr) {}

    ~ReturnStatement() override;

    void compile(CompilationState &state) override;
};

struct IOStatement : Statement {

    enum class IOFunction {
        IOINPUT, IOOUTPUT
    } function;

    Expression *expr;

    IOStatement(IOFunction function, Expression *expr)
            : function(function), expr(expr) {}

    ~IOStatement() override {
        delete expr;
        expr = nullptr;
    }

    void compile(CompilationState &state) override;
};

struct ExpressionStatement : Statement {
	Expression *expr;

    explicit ExpressionStatement(Expression *expr) : expr(expr) {}

    ~ExpressionStatement() override {
        delete expr;
        expr = nullptr;
    }

    void compile(CompilationState &state) override;
};

struct InlineStatement : Statement {
	string *inl;

    explicit InlineStatement(string *inl) : inl(inl) {}

    ~InlineStatement() override;

    void compile(CompilationState &state) override;
};

struct ListStatement : Statement {
    Symbol *function;
    vector<Statement*> *list;

    ListStatement(vector<Statement*> *list);

    ~ListStatement() override;

    void compile(CompilationState &state) override;
};

struct TupleExpression : Expression {
    vector<Expression*> tuple;

    TupleExpression(Expression *lhs, Expression *rhs);

    ~TupleExpression() override;

    void compile(CompilationState &state) override;
};

struct StringExpression : Expression {
    std::string *string;

    StringExpression(std::string *str);
    ~StringExpression() override;
    void compile(CompilationState &state) override;
};

#endif // BF_H