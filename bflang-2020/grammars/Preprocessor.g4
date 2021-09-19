grammar Preprocessor;

main: '#' directive args? value? NEWLINE;

directive: IDENTIFIER;

args: '(' (IDENTIFIER (',' IDENTIFIER)*)? ')';

value: UNTIL_NEWLINE;

UNTIL_NEWLINE: [^\n]+;

NEWLINE: '\n'+;

IDENTIFIER: [A-Z][A-Z0-9]*;

WS: [ \t\r\n]+ -> skip;