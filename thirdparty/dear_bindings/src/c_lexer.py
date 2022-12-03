import ply.lex as lex
from src import token_stream

# This implements a simple lexer for C

states = [
    ('pp', 'exclusive'),  # State when parsing preprocessor directive content (but not the initial directive)
]

# List of recognised preprocessor command tokens (except pragma, which is a special case)
preprocessor_commands = {
    '#ifdef': 'PPIFDEF',
    '#ifndef': 'PPIFNDEF',
    '#if': 'PPIF',
    '#else': 'PPELSE',
    '#elif': 'PPELIF',
    '#endif': 'PPENDIF',
    '#undef': 'PPUNDEF',
    '#include': 'PPINCLUDE'
}

# List of reserved words we want to turn into tokens
reserved_words = {
    'struct': 'STRUCT',
    'class': 'CLASS',
    'union': 'UNION',
    'typedef': 'TYPEDEF',
    'signed': 'SIGNED',
    'unsigned': 'UNSIGNED',
    'const': 'CONST',
    'constexpr': 'CONSTEXPR',
    'namespace': 'NAMESPACE',
    'enum': 'ENUM',
    'template': 'TEMPLATE'
}

tokens = [
    'LINE_COMMENT',
    'BLOCK_COMMENT',
    'THING',
    'STRING_LITERAL',
    'CHARACTER_LITERAL',
    'DECIMAL_LITERAL',
    'HEX_LITERAL',
    'OCTAL_LITERAL',
    'FLOAT_LITERAL',
    'BOOL_LITERAL',
    'POINTER_LITERAL',
    'KEYWORD',
    'SEMICOLON',
    'COLON',
    'NEWLINE',
    'WHITESPACE',
    'LPAREN',
    'RPAREN',
    'LBRACE',
    'RBRACE',
    'LSQUARE',
    'RSQUARE',
    'LTRIANGLE',
    'RTRIANGLE',
    'ASTERISK',
    'AMPERSAND',
    'COMMA',
    'PRAGMA',
    'PPERROR',
    'EQUAL',
    'ELLIPSES',
    'LOGICALAND',
    'LOGICALOR',
    'PREPROCESSOR_COMMAND',
    'PPDEFINE',
    'PPDEFINED',
    'PPNOT',
    'PPLPAREN',
    'PPRPAREN',
    'PPEQUAL',
    'PPNOTEQUAL',
    'PPLESS',
    'PPLESSEQUAL',
    'PPGREATER',
    'PPGREATEREQUAL',
    'PPOR',
    'PPAND',
    'PPSYSFILENAME_LITERAL',
    'PPTHING',
 ] + list(preprocessor_commands.values()) + list(reserved_words.values())

literals = ['!', '/', '\\', '~', '+', '-', '^', '=', '%', '|', '.', '<', '>', '(', ')', '[', ']', '{', '}', '?']

t_ANY_LINE_COMMENT = r'\/\/.*'  # // Comment
t_ANY_BLOCK_COMMENT = r'/\*([\s\S]*?)\*/'  # /* Comment */
t_ANY_STRING_LITERAL = r'".*?"'  # Any string literal
t_ANY_CHARACTER_LITERAL = r'\'\\?.\''  # Any single-character literal
t_ANY_DECIMAL_LITERAL = r'[+-]?[0-9][0-9]*[Uu]?[Ll]?[Ll]?'  # A non-prefixed decimal number, with an optional suffix
t_ANY_HEX_LITERAL = r'[+-]?0[xX][0-9A-Fa-f]*[Uu]?[Ll]?[Ll]?'  # A prefixed hexidecimal number, with an optional suffix
t_ANY_OCTAL_LITERAL = r'[+-]?0[0-7]*[Uu]?[Ll]?[Ll]?'  # A prefixed octal number, with an optional suffix
t_ANY_FLOAT_LITERAL = r'[+-]?[0-9]*\.[0-9]*[eE]?[+-]?[0-9]*[FfLl]?'  # A decimal number, with an optional suffix
t_ANY_POINTER_LITERAL = r'\bnullptr\b'  # A pointer literal (the only thing defined for this in the standard is nullptr)
t_SEMICOLON = r';'  # Line terminator
t_COLON = r':'
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_LBRACE = r'\{'
t_RBRACE = r'\}'
t_LSQUARE = r'\['
t_RSQUARE = r'\]'
t_LTRIANGLE = r'\<'
t_RTRIANGLE = r'\>'
t_ASTERISK = r'\*'
t_AMPERSAND = r'&'
t_EQUAL = r'='
t_COMMA = r','
t_LOGICALAND = r'\&\&'
t_LOGICALOR = r'\|\|'
# C++ reserved keywords
t_KEYWORD = r'\b(alignas|alignof|asm|auto|break|case|catch|char|constexpr|'\
            r'const_cast|continue|decltype|default|delete|do|doubledynamic_cast|else|enum|explicit|export|extern|'\
            r'float|for|friend|goto|if|inline|int|long|mutable|namespace|new|noexcept|operator|private|protected|'\
            r'public|register|reinterpret_cast|return|short|sizeof|static|static_assert|static_cast|'\
            r'switch|template|this|thread_local|throw|try|typedef|typeid|typename|union|using|virtual|void|'\
            r'volatile|wchar_t|while)\b'


# Ellipses (function for priority)
def t_ELLIPSES(t):
    r'\.\.\.'
    return t


# A boolean literal (function for priority)
def t_ANY_BOOL_LITERAL(t):
    r'\b(true|false)\b'
    return t


# Match #pragma
# This is a special-case because there can be all sorts of random stuff after #pragma and we want to eat it all
def t_PRAGMA(t):
    r'(?m)^\#pragma.+?(?=(\/\/|\/\*|$))'
    return t


# Match #error
# This is a special-case because there can be all sorts of random stuff after #error and we want to eat it all
def t_PPERROR(t):
    r'(?m)^\#error.+?(?=(\/\/|\/\*|$))'
    return t


# Match #define
# This is a special-case because there can be all sorts of random stuff after #define and we want to eat it all
def t_PPDEFINE(t):
    r'(?m)^\#define.+?(?=(\/\/|\/\*|$))'
    return t


# Match any preprocessor command
def t_PREPROCESSOR_COMMAND(t):
    r'(?m)^\#[A-Za-z_][0-9A-Za-z_]*'
    t.type = preprocessor_commands.get(t.value, 'PREPROCESSOR_COMMAND')
    t.lexer.begin('pp') # The rest of the line will be parsed in the preprocessor state
    return t

# Match any identifier-like "thing" (also turns reserved words into their appropriate token)
def t_THING(t):
    r'[A-Za-z_][0-9A-Za-z_]*'
    t.type = reserved_words.get(t.value, 'THING')
    return t

# Keep track of line numbers
def t_NEWLINE(t):
    r'[\n\r]'
    t.lexer.lineno += len(t.value)
    return t


# Whitespace
def t_WHITESPACE(t):
    r'[ \t]+'
    return t

#t_ignore = ' \t'


# Error handling
def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)

# Preprocessor state tokens

t_pp_PPNOT = r'\!'
t_pp_PPLPAREN = r'\('
t_pp_PPRPAREN = r'\)'
t_pp_PPOR = r'\|\|'
t_pp_PPAND = r'&&'
t_pp_PPEQUAL = r'=='
t_pp_PPNOTEQUAL = r'!='
t_pp_PPLESS = r'<'
t_pp_PPLESSEQUAL = r'<='
t_pp_PPGREATER = r'>'
t_pp_PPGREATEREQUAL = r'>='
t_pp_PPSYSFILENAME_LITERAL = r'<.*>'  # A system header filename in an include
t_pp_PPTHING = r'[A-Za-z_][0-9A-Za-z_]*' # An identifier-like thing in the preprocessor


# Catch "defined" as a keyword - this has to be a function for priority reasons
def t_pp_PPDEFINED(t):
    r'defined'
    return t


def t_pp_NEWLINE(t):
    r'[\n\r]+'
    t.lexer.lineno += len(t.value)
    t.lexer.begin('INITIAL')  # Back to normal parsing
    return t


t_pp_ignore = ' \t'


def t_pp_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)


# Lex a given source (string) and return a token stream for it
def tokenize(source):
    lexer = lex.lex()
    lexer.input(source)
    return token_stream.TokenStream(lexer)
