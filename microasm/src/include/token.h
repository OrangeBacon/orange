#ifndef TOKEN_H
#define TOKEN_H

#define FOREACH_TOKEN(x) \
    x(LEFT_PAREN) x(RIGHT_PAREN) \
    x(LEFT_BRACE) x(RIGHT_BRACE) \
    x(COMMA) x(DOT) x(COLON) x(NUMBER) \
    x(SEMICOLON) x(STAR) x(EQUAL) \
    x(IDENTIFIER) x(OPCODE) x(HEADER) \
    x(MACRO) x(INPUT) x(OUTPUT) \
    x(ERROR) x(EOF) x(NULL)

#define ENUM_TOKEN(x) TOKEN_##x,
#define ADD_TOKEN(x) +1

typedef enum OrangeTokenType {
    FOREACH_TOKEN(ENUM_TOKEN)
} OrangeTokenType;

const char* TokenNames[FOREACH_TOKEN(ADD_TOKEN)];

#undef ENUM_TOKEN
#undef ADD_TOKEN

typedef union TokenData {
    const char* string;
    unsigned int value;
} TokenData;

// type and source location of a token
typedef struct Token {
    OrangeTokenType type;
    const char* base; // source file buffer
    int offset;
    int length;
    int line;
    int column;
    TokenData data;
} Token;

#define TOKEN_GET(token) \
    ((const char*)((token).base + (token).offset))

// output the representation of a token to stdout
void TokenPrint(Token* token);

#endif