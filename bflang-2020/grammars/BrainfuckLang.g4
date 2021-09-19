grammar BrainfuckLang;

main: imports* topLevelScope
    | EOF
    ;

imports: 'import' string ';';

statement: declarationStatement
         | bfStatement
         | asmStatement
         | readStatement
         | writeStatement
         | jumpStatement
         | usingStatement
         | blockStatement
         | expressionStatement
         ;

declarationStatement: variableDeclarationStatement
                    | functionDeclarationStatement
                    | classDeclarationStatement
                    | aliasDeclarationStatement
                    | enumDeclarationStatement
                    | templateDeclarationStatement
                    | namespaceDeclarationStatement
                    ;

topLevelScope: (declarationStatement | ';')*;

variableDeclarationStatement: variableQualifier+ variableDeclarationList ';';

variableDeclarationList: variableDeclaration (',' variableDeclaration)*;

variableDeclaration: (IDENTIFIER ':')? typeSignature variableInitialization?;

variableInitialization: '=' expr;

typeSignature: typeNameTypeSignature
             | functionPointerTypeSignature
             ;

typeNameTypeSignature: typeQualifier* typeName typeQualifier* pointer*;

functionPointerTypeSignature: '*' functionSignature;

typeName: isVoid='void'
        | isAuto='auto'
        | nativeTypeName
        | qualifiedIdTypeName
        | classDeclarationTypeName
        ;

qualifiedIdTypeName: qualifiedId;

nativeTypeName: typeSign | typeSign? nativeTypeNames;

classDeclarationTypeName: classDeclaration classImplementation;

typeSign: isUnsigned='unsigned' | isSigned='signed';

nativeTypeNames: isChar='char' | isBool='bool' | isCell='cell';

pointer: ('[' expr? ']' | '*') typeQualifier*;

aliasDeclarationStatement: aliasDeclaration '=' typeSignature ';';

aliasDeclaration: 'alias' IDENTIFIER;

enumDeclarationStatement: enumDeclaration (enumImplementation | isForwardDeclaration=';');

enumDeclaration: 'enum' IDENTIFIER;

enumImplementation: '{' (enumerationItem (',' enumerationItem)*)? '}';

enumerationItem: IDENTIFIER ('=' expr)?;

namespaceDeclarationStatement: 'namespace' IDENTIFIER '{' topLevelScope '}';

classDeclarationStatement: classDeclaration (classImplementation | isForwardDeclaration=';');

classDeclaration: (isClass='class' | isStruct='struct' | isInterface='interface') qualifiedId?;

classImplementation: (':' inheritanceList)? '{' topLevelScope '}';

inheritanceList: qualifiedId (',' qualifiedId)*;

functionDeclarationStatement: functionDeclaration (functionImplementation | isForwardDeclaration=';');

functionDeclaration: functionSpecifier* ('fun' | 'def') qualifiedId functionSignature;

functionSignature: functionParameters functionQualifier* ('->' functionReturnType)?;

functionImplementation: '{' (statement | ';')* '}';

functionParameters: '(' isVoid='void' ')'
                  | '(' variableDeclarationList? ')'
                  ;

functionReturnType: isVoid='void'
                  | variableDeclarationList
                  | '(' isNested=functionReturnType ')'
                  ;

bfStatement: '__bf' string ';';

asmStatement: '__asm' string ';';

readStatement: '__read' argumentExpressionList ';';

writeStatement: '__write' argumentExpressionList ';';

jumpStatement: ifStatement
             | whileStatement
             | doStatement
             | returnStatement
             | breakStatement
             | continueStatement
             ;

ifStatement: 'if' '(' expr ')' statement ('else' statement)?;

whileStatement: statementLabelDeclaration? 'while' '(' expr ')' statement;

doStatement: statementLabelDeclaration? 'do' statement 'while' '(' expr ')' ';';

statementLabelDeclaration: statementLabel ':';

returnStatement: 'return' argumentExpressionList? ';'
               | 'return' '(' argumentExpressionList? ')' ';'
               ;

breakStatement: 'break' statementLabel? ';';

continueStatement: 'continue' statementLabel? ';';

statementLabel: labelName=IDENTIFIER;

usingStatement: 'using' 'namespace' qualifiedId ';';

blockStatement: '{' (statement | ';')* '}';

expressionStatement: expr ';';

expr: atom # AtomicExpression
    | expr suffixOperator # SuffixExpression
    | prefixOperator expr # PrefixExpression
    | lhs=expr (mul='*' | div='/' | mod='%') rhs=expr # MulDivModExpression
    | lhs=expr (add='+' | sub='-') rhs=expr # AddSubExpression
    | lhs=expr (left='<<' | right='>>') rhs=expr # BitShiftExpression
    | lhs=expr (le='<' | leq='<=' ) rhs=expr # LessExpression
    | lhs=expr (gr='>' | geq='>=') rhs=expr # GreaterExpression
    | lhs=expr (eq='==' | neq='!=') rhs=expr # EqualityExpression
    | lhs=expr '&' rhs=expr # BitAndExpression
    | lhs=expr '^' rhs=expr # BitXorExpression
    | lhs=expr '|' rhs=expr # BitOrExpression
    | lhs=expr '&&' rhs=expr # LogicAndExpression
    | lhs=expr '||' rhs=expr # LogicOrExpression
    | condition=expr '?' onTrue=expr ':' onFalse=expr # TernaryExpression
    | <assoc=right> lhs=expr assignmentOperator rhs=expr # AssignmentExpression
    | '(' expr ')' # GroupExpression
    | '(' expr (',' expr)* ',' ')' # CompoundExpression
    ;

atom: 'this' # ThisExpression
    | constant # ConstantExpression
    | qualifiedId # IdExpression
    | compilerMacro # MacroExpression
    ;

compilerMacro: COMPILER_MACRO_FUNCTION
             | COMPILER_MACRO_FILE
             | COMPILER_MACRO_LINE
             ;

assignmentOperator: (assign='=' | add='+=' | sub='-=' | mul='*=' | div='/=' | mod='%=' | left='<<=' | right='>>=' | bitAnd='&=' | bitOr='|=' | bitXor='^=' | logAnd='&&=' | logOr='||=');

suffixOperator: ( incrementOperator='++'
                | decrementOperator='--'
                | callSuffix
                | subscriptSuffix
                | functionalCastSuffix
                | memberAccessOperator=('->' | '.') memberId=qualifiedId
                );

callSuffix: '(' argumentExpressionList? ')';

subscriptSuffix: '[' argumentExpressionList? ']';

functionalCastSuffix: '{' argumentExpressionList? '}';

argumentExpressionList: expr (',' expr)*;

prefixOperator: (dec='--' | inc='++' | pos='+' | neg='-' | logInvert='!' | bitInvert='~' | typeCast | dereference='*' | addressOf='&' | sizeOf='sizeof');

typeCast: '(' typeSignature ')';

templateInstantiation: '<' templateArgumentList? '>';

templateArgumentList: templateInstantiationArgument (',' templateInstantiationArgument)*;

templateInstantiationArgument: typeSignature
                             | expr
                             ;

qualifiedId: unqualifiedId ('::' unqualifiedId)*;

unqualifiedId: unqualifiedPrimaryId
             | unqualifiedOperatorId
             | unqualifiedTemplateId
             ;

unqualifiedPrimaryId: '~'? IDENTIFIER;

unqualifiedOperatorId: 'operator' (anyOperator | qualifiedId);

unqualifiedTemplateId: IDENTIFIER templateInstantiation;

constant: integer | string;

string: STRING+;

templateDeclarationStatement: 'template' '<' templateParameterDeclarationList? '>' templateable;

templateParameterDeclarationList: templateParameterDeclaration (',' templateParameterDeclaration)*;

templateable: aliasDeclarationStatement
            | functionDeclarationStatement
            | classDeclarationStatement
            | templateDeclarationStatement
            ;

templateParameterDeclaration: constexprTemplateParameterDeclaration
                            | typenameTemplateParameterDeclaration
                            ;

constexprTemplateParameterDeclaration: 'constexpr' variableDeclaration ('=' expr)?;

typenameTemplateParameterDeclaration: 'typename' paramName=IDENTIFIER ('=' typeSignature)?;

variableQualifier: isStatic='static'
                 | isConst='const'
                 | isConstexpr='constexpr'
                 | isVariable='var'
                 ;

typeQualifier: isConst='const'
             | isConstexpr='constexpr'
             ;

functionSpecifier: isStatic='static'
                 | isAbstract='abstract'
                 | isOverride='override'
                 | isGetter='get'
                 | isSetter='set'
                 | isInline='inline'
                 ;

functionQualifier: isConst='const'
                 | isConstexpr='constexpr'
                 ;

integer: DECIMAL # DecimalIntegerExpression
       | HEX # HexIntegerExpression
       | OCT # OctIntegerExpression
       | BIN # BinIntegerExpression
       | CHARACTER # CharIntegerExpression
       | 'true' # TrueIntegerExpression
       | 'false' # FalseIntegerExpression
       | 'nullptr' # NullptrIntegerExpression
       ;

anyOperator: '++'
         | '--'
         | '+'
         | '-'
         | '*'
         | '/'
         | '%'
         | '->'
         | '.'
         | '>='
         | '>'
         | '<='
         | '<'
         | '&&'
         | '||'
         | '!'
         | '<<'
         | '>>'
         | '&'
         | '|'
         | '^'
         | '~'
         | '=='
         | '!='
         | '='
         | '+='
         | '-='
         | '*='
         | '/='
         | '%='
         | '&&='
         | '||='
         | '&='
         | '|='
         | '^='
         | '~='
         | '<<='
         | '>>='
         | ('(' ')')
         | ('[' ']')
         | ('<' '>')
         ;

// Keywords
IF: 'if';
WHILE: 'while';
// DO: 'do';
// FOR: 'for';
VOID: 'void';
CLASS: 'class';
STRUCT: 'struct';
FUN: 'fun';
DEF: 'def';
INTERFACE: 'interface';
VAR: 'var';
TRUE: 'true';
FALSE: 'false';
NULLPTR: 'nullptr';
THIS: 'this';
AUTO: 'auto';
READ: '__read';
WRITE: '__write';
SIZEOF: 'sizeof';
ALIAS: 'alias';
IMPORT: 'import';
NAMESPACE: 'namespace';
CELL: 'cell';
INTEGER: 'int';
CHAR: 'char';
BOOL: 'bool';
OPERATOR: 'operator';
BF: '__bf';
ASM: '__asm';
USING: 'using';
CONTINUE: 'continue';
BREAK: 'break';
RETURN: 'return';
ENUM: 'enum';
STATIC: 'static';
CONSTEXPR: 'constexpr';
INLINE: 'inline';
CONST: 'const';
GET: 'get';
SET: 'set';
ABSTRACT: 'abstract';
OVERRIDE: 'override';
TEMPLATE: 'template';
TYPENAME: 'typename';
SIGNED: 'signed';
UNSIGNED: 'unsigned';
COMPILER_MACRO_FUNCTION: '__FUNCTION__';
COMPILER_MACRO_LINE: '__LINE__';
COMPILER_MACRO_FILE: '__FILE__';

// Operator keywords
INC: '++';
DEC: '--';
PLUS: '+';
MINUS: '-';
ASTERISK: '*';
BSLASH: '/';
MOD: '%';
INDIRECTION: '->';
DOT: '.';
GREATER_EQUALS: '>=';
GREATER: '>';
LESSER_EQUALS: '<=';
LESS: '<';
LOGIC_AND: '&&';
LOGIC_OR: '||';
EXCLAMATION: '!';
SHIFT_LEFT: '<<';
SHIFT_RIGHT: '>>';
AMPERSAND: '&';
PIPE: '|';
HAT: '^';
TILDE: '~';
EQ: '==';
NEQ: '!=';
DOUBLE_COLON: ':';
SCOPE: '::';
ASSIGNMENT_OP: '=';
ASSIGN_ADD_OP: '+=';
ASSIGN_SUB_OP: '-=';
ASSIGN_MUL_OP: '*=';
ASSIGN_DIV_OP: '/=';
ASSIGN_MOD_OP: '%=';
ASSIGN_LOG_AND_OP: '&&=';
ASSIGN_LOG_OR_OP: '||=';
ASSIGN_BIT_AND_OP: '&=';
ASSIGN_BIT_OR_OP: '|=';
ASSIGN_BIT_XOR_OP: '^=';
ASSIGN_BIT_NEG_OP: '~=';
ASSIGN_SHIFT_LEFT: '<<=';
ASSIGN_SHIFT_RIGHT: '>>=';
BRACE_OPEN: '(';
BRACE_CLOSED: ')';
BRACKET_OPEN: '[';
BRACKET_CLOSED: ']';

//ignored comments
MULTILINE_COMMENT: '/*' .*? ('*/' | EOF) { skip(); };
COMMENT: '//' .*? ('\n' | EOF) { skip(); };

// ignored whitespace
WS: [ \t\r\n]+ { skip(); };

// Numbers
DECIMAL: [1-9][0-9]* | '0'+;
HEX: '0x' [0-9a-fA-F]+;
OCT: '0' [1-7][0-7]*;
BIN: '0b' [01]+;
CHARACTER: '\'' ('\\n' | '\\r' | '\\t' | .) '\'';

// String
STRING: '"' .*? '"';

// Any valid symbol identifier
IDENTIFIER: [_a-zA-Z][_a-zA-Z0-9]*;

// Unknown character
// todo: exception
ERRCHAR: . { skip(); };