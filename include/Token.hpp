#pragma once
#include <string>

#define MAX_IDENTIFIER_LENGTH 256
#define MAX_DIGIT_LENGTH 256

typedef struct identifier_value {
        char identifier_string[MAX_IDENTIFIER_LENGTH];
} identifier_value_t;




typedef enum{
        T_UNKNOWN,
        T_EOF,
        T_IDENTIFIER,
        T_SYM_LPAREN,
        T_SYM_RPAREN,
        T_SYM_LBRACE,
        T_SYM_RBRACE,
        T_SYM_LBRACKET,
        T_SYM_RBRACKET,
        T_SYM_SEMICOLON,
        T_SYM_COLON,
        T_SYM_PERIOD,
        T_SYM_COMMA,
        T_OP_LOGI_AND,
        T_OP_LOGI_OR,
        T_OP_LOGI_NOT,
        T_OP_ASIGN_EQUALS,
        T_OP_TERM_MULTIPLY,
        T_OP_TERM_DIVIDE,
        T_OP_REL_GREATER,
        T_OP_REL_LESS,
        T_OP_REL_GREATER_EQUAL,
        T_OP_REL_LESS_EQUAL,
        T_OP_REL_EQUAL,
        T_OP_REL_NOT_EQUAL,
        T_OP_ARITH_PLUS,
        T_OP_ARITH_MINUS,
        T_CONST_INTEGER,
        T_CONST_FLOAT,
        T_CONST_STRING,
        T_RW_PROGRAM, // DANGER: if any T_RW values are added or removed update scanner.c - check_reserved_word to match.
        T_RW_IS,
        T_RW_BEGIN,
        T_RW_END,
        T_RW_GLOBAL,
        T_RW_INTEGER,
        T_RW_FLOAT,
        T_RW_STRING,
        T_RW_BOOL,
        T_RW_ENUM,
        T_RW_PROCEDURE,
        T_RW_VARIABLE,
        T_RW_IF,
        T_RW_THEN,
        T_RW_ELSE,
        T_RW_FOR,
        T_RW_RETURN,
        T_RW_NOT, // TODO What to do about this? Technically both reserved word and bool operator.
        T_RW_TYPE,
        T_RW_TRUE,
        T_RW_FALSE,
} token_type_e;

class token_t{
public:
        token_t();
        ~token_t();
        unsigned int line_num;
        token_type_e type;

        void setValue(std::string value);
        void setValue(int value);
        void setValue(float value);

        void destroy();

        std::string* getStringValue();
        int getIntValue();
        float getFloatValue();
private:
        enum value_type_e {
                NONE,
                STRING,
                INT,
                FLOAT
        };
        enum value_type_e value_type;

        void* _value;
        void* tag; // Used to prevent double free

};

void print_token(token_t token);