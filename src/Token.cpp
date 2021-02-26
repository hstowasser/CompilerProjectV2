#include "Token.hpp"
#include <stdio.h>

static const char * token_type_to_string(token_type_e token_type)
{
        switch (token_type){
        case T_IDENTIFIER:
                return "T_IDENTIFIER";
        case T_EOF:
                return "T_EOF";
        case T_SYM_LPAREN:
                return "T_SYM_LPAREN";
        case T_SYM_RPAREN:
                return "T_SYM_RPAREN";
        case T_SYM_LBRACE:
                return "T_SYM_LBRACE";
        case T_SYM_RBRACE:
                return "T_SYM_RBRACE";
        case T_SYM_LBRACKET:
                return "T_SYM_LBRACKET";
        case T_SYM_RBRACKET:
                return "T_SYM_RBRACKET";
        case T_SYM_SEMICOLON:
                return "T_SYM_SEMICOLON";
        case T_SYM_COLON:
                return "T_SYM_COLON";
        case T_SYM_PERIOD:
                return "T_SYM_PERIOD";
        case T_SYM_COMMA:
                return "T_SYM_COMMA";
        case T_OP_LOGI_AND:
                return "T_OP_LOGI_AND";
        case T_OP_LOGI_OR:
                return "T_OP_LOGI_OR";
        case T_OP_LOGI_NOT:
                return "T_OP_LOGI_NOT";
        case T_OP_ASIGN_EQUALS:
                return "T_OP_ASIGN_EQUALS";
        case T_OP_TERM_MULTIPLY:
                return "T_OP_TERM_MULTIPLY";
        case T_OP_TERM_DIVIDE:
                return "T_OP_TERM_DIVIDE";
        case T_OP_REL_GREATER:
                return "T_OP_REL_GREATER";
        case T_OP_REL_LESS:
                return "T_OP_REL_LESS";
        case T_OP_REL_GREATER_EQUAL:
                return "T_OP_REL_GREATER_EQUAL";
        case T_OP_REL_LESS_EQUAL:
                return "T_OP_REL_LESS_EQUAL";
        case T_OP_REL_EQUAL:
                return "T_OP_REL_EQUAL";
        case T_OP_REL_NOT_EQUAL:
                return "T_OP_REL_NOT_EQUAL";
        case T_OP_ARITH_PLUS:
                return "T_OP_ARITH_PLUS";
        case T_OP_ARITH_MINUS:
                return "T_OP_ARITH_MINUS";
        case T_CONST_INTEGER:
                return "T_CONST_INTEGER";
        case T_CONST_FLOAT:
                return "T_CONST_FLOAT";
        case T_CONST_STRING:
                return "T_CONST_STRING";
        case T_RW_PROGRAM:
                return "T_RW_PROGRAM";
        case T_RW_IS:
                return "T_RW_IS";
        case T_RW_BEGIN:
                return "T_RW_BEGIN";
        case T_RW_END:
                return "T_RW_END";
        case T_RW_GLOBAL:
                return "T_RW_GLOBAL";
        case T_RW_INTEGER:
                return "T_RW_INTEGER";
        case T_RW_FLOAT:
                return "T_RW_FLOAT";
        case T_RW_STRING:
                return "T_RW_STRING";
        case T_RW_BOOL:
                return "T_RW_BOOL";
        case T_RW_ENUM:
                return "T_RW_ENUM";
        case T_RW_PROCEDURE:
                return "T_RW_PROCEDURE";
        case T_RW_VARIABLE:
                return "T_RW_VARIABLE";
        case T_RW_IF:
                return "T_RW_IF";
        case T_RW_THEN:
                return "T_RW_THEN";
        case T_RW_ELSE:
                return "T_RW_ELSE";
        case T_RW_FOR:
                return "T_RW_FOR";
        case T_RW_RETURN:
                return "T_RW_RETURN";
        case T_RW_NOT:
                return "T_RW_NOT";
        case T_RW_TYPE:
                return "T_RW_TYPE";
        case T_RW_TRUE:
                return "T_RW_TRUE";
        case T_RW_FALSE:
                return "T_RW_FALSE";
        default:
                return "T_UNKNOWN";
        }
}

void print_token(token_t token)
{
        printf("Type: %s ", token_type_to_string(token.type));
        if ( token.type == T_IDENTIFIER || token.type == T_CONST_STRING){
                printf("value: %s", token.getStringValue()->c_str());
        }else if ( token.type == T_CONST_INTEGER) {
                printf("integer value: %d", token.getIntValue());
        }else if ( token.type == T_CONST_FLOAT) {
                printf("float value: %f", token.getFloatValue());
        }
        printf("\n");
}

token_t::token_t()
{
        this->line_num = 0;
        this->type = T_UNKNOWN;
        this->_value = NULL;
        this->tag = NULL;
        this->value_type = NONE;
}

token_t::~token_t()
{

}

void token_t::destroy()
{
        if ( this->value_type == INT || this->value_type == FLOAT){
                free(this->_value);
        }else if ( this->value_type == STRING){
                delete (std::string*)(this->_value);
        }
}

void token_t::setValue(std::string value)
{
        if ( this->_value == NULL){
                this->_value = (void*) new std::string(value);
                this->value_type = STRING;
                this->tag = (void*)this;
        }
}

void token_t::setValue(int value)
{
        if ( this->_value == NULL){
                this->_value = malloc(sizeof(int));
                if ( this->_value != NULL){
                        *((int*)this->_value) = value;
                        this->value_type = INT;
                        this->tag = (void*)this;
                }
        }
}

void token_t::setValue(float value)
{
        if ( this->_value == NULL){
                this->_value = malloc(sizeof(float));
                if ( this->_value != NULL){
                        *((float*)this->_value) = value;
                        this->value_type = FLOAT;
                        this->tag = (void*)this;
                }
        }
}

std::string* token_t::getStringValue()
{
        if (this->value_type == STRING){
                return (std::string*)this->_value;
        }else{
                //return ""; // Error
                return NULL;
        }
}

int token_t::getIntValue()
{
        if (this->value_type == INT){
                return *((int*)this->_value);
        }else{
                return 0; // Error
        }
}

float token_t::getFloatValue()
{
        if (this->value_type == FLOAT){
                return *((float*)this->_value);
        }else{
                return 0; // Error
        }
}
