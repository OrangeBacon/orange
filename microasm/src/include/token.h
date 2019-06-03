#ifndef TOKEN_H
#define TOKEN_H

#define FOREACH_TOKEN(x) \
    x(TOKEN_LEFT_PAREN) x(TOKEN_RIGHT_PAREN) \
    x(TOKEN_LEFT_BRACE) x(TOKEN_RIGHT_BRACE) \
    x(TOKEN_COMMA) x(TOKEN_DOT) x(TOKEN_COLON) \
    x(TOKEN_SEMICOLON) x(TOKEN_STAR) x(TOKEN_EQUAL) \
    x(TOKEN_IDENTIFIER) x(TOKEN_NUMBER) \
    x(TOKEN_ERROR) x(TOKEN_EOF)

#define ENUM_TOKEN(x) x,
#define STRING_TOKEN(x) #x,
#define ADD_TOKEN(x) +1
#define X_MACRO_LENGTH(x) 0 x(ADD_TOKEN)

typedef enum TokenType {
    FOREACH_TOKEN(ENUM_TOKEN)
} TokenType;

const char* TokenNames[X_MACRO_LENGTH(FOREACH_TOKEN)];

#undef ENUM_TOKEN
#undef ADD_TOKEN
#undef X_MACRO_LENGTH

// type and source location of a token
typedef struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;
    int column;
} Token;

// output the representation of a token to stdout
void TokenPrint(Token* token);

#endif