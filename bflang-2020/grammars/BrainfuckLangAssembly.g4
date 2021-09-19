grammar BrainfuckLangAssembly;
//ignored comments
MULTILINE_COMMENT: '/*' .*? '*/' { skip(); };
COMMENT: ';' .*? '\n' { skip(); };
// ignored whitespace
WS: [ \t\r\n]+ { skip(); };
// identifier
IDENTIFIER: [_a-zA-Z][_a-zA-Z0-9]*;
// various ways to write numbers
DEC: [1-9][0-9]* | '0'+;
HEX: '0x' [0-9a-fA-F]+;
OCT: '0' [1-7][0-7]*;
BIN: '0b' [01]+;
// used to address the pointer register of a cell
REGISTER_OP: '^';
// scope dereference operator
DESCOPE_OP: '::';
// instance dereference operator
DEREFERENCE_OP: '.';
// Integer value operator
INTEGER_OP: '$';
// Operator for addresses
ADDRESS_OP: '@';
// Operator for signed values
SIGN_OP: '-' | '+';
// data regex
DATA_TEXT_ENTRY: '\'' .*? '\'' | '"' .*? '"';

// todo: add text section
main: symbolTableSection codeSection;

symbolTableSection: symbolTableEntry+;

symbolTableEntry: labelSymbolEntry
                  | functionSymbolEntry
                  | variableSymbolEntry
                  | typeSymbolEntry
                  ;

// a label that can be jumped to
labelSymbolEntry: 'label' symbolId;

// matching path in the symbol table
symbolId: IDENTIFIER descope* dereference*;

// symbol scope
descope: DESCOPE_OP IDENTIFIER;

// dereferences an variable instance
dereference: DEREFERENCE_OP IDENTIFIER;

// holds data needed to create a function stack
// todo: argument and return list do not make sense! need symbol id in signature?
functionSymbolEntry: 'function' symbolId argumentList? returnList?;

// lists the types of the arguments
argumentList: '(' variableTypeList ')';

// list the types of the return variables
returnList: '->' variableTypeList;

// a comma separated list of variables types
variableTypeList: variableType ( ',' variableType)*;

// a variable type is written by giving a symbol name and an optional array length in brackets
variableType: symbolId | variableType arraySize;

// array size is either an integer in brackets or unspecified by a star
arraySize: '[' integer ']' | '*';

// a variable symbols consists of the name of the variable, the address on the stack, as well as the type
variableSymbolEntry: 'variable' variableType symbolId;

// addresses are either local with an offset or global
address: ADDRESS_OP (localAddress | globalAddress);

// A local address defined as an offset
localAddress: SIGN_OP integer;

// A global address has no offset indicator
globalAddress: integer;

// type symbols are simply defined by a name inside a scope
typeSymbolEntry: 'type' symbolId;

// begin of the code section
codeSection: label+;

// a label is a label identifier followed by instructions
label: symbolId ':' instruction+;

// instructions are a command with arguments
instruction: instructionName (argument (',' argument)*)?;

// possible command tokens
instructionName: 'read'
   | 'write'
   | 'add'
   | 'sub'
   | 'inc'
   | 'dec'
   | 'mul'
   | 'div'
   | 'jmp'
   | 'jnz'
   | 'jez'
   | 'cmp'
   | 'mov'
   | 'la'
   | 'call'
   | 'ret'
   | 'nop'
   ;

// an instruction argument
argument: lvalue | rvalue | cvalue;

// reference values; evaluated at compile time
lvalue: (symbolId | address) REGISTER_OP?;

// dereferenced value; computed at runtime time
rvalue: '[' lvalue scale? coffset? ']';

// constant scaled variable offset
scale: SIGN_OP (integer '*')? lvalue;

// constant offset
coffset: SIGN_OP integer;

// constant value
cvalue: INTEGER_OP integer;

integer: DEC | HEX | OCT | BIN;
