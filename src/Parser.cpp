#include "Parser.hpp"
#include "Token.hpp"

#if 0
#include "stdio.h"
#define debug_print_call() printf("%s\n", __FUNCTION__);
#else
#define debug_print_call()
#endif

#if 1
#define debug_print_token(itr) print_token(itr)
#else
#define debug_print_token(itr)
#endif



Parser::Parser(/* args */)
{
}

Parser::~Parser()
{
}

void Parser::inc_ptr(std::list<token_t>::iterator *itr)
{
        if (*itr != this->itr_end) {
                (*itr)++;
        }else{
                // TODO Ahhhhh???
        }
}

bool Parser::parseExpression(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        // check for not
        if ((*itr)->type == T_OP_LOGI_NOT){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }
        
        ret = this->parseArithOp(itr);
        if (ret){
                if (((*itr)->type == T_OP_LOGI_AND) ||
                    ((*itr)->type == T_OP_LOGI_OR))
                {
                        debug_print_token(**itr);
                        // Then it's an Expression?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseExpression(itr);
                }
        }
        return ret;
}

bool Parser::parseArithOp(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        ret = this->parseRelation(itr);
        if (ret){
                if (((*itr)->type == T_OP_ARITH_MINUS) ||
                    ((*itr)->type == T_OP_ARITH_PLUS))
                {
                        debug_print_token(**itr);
                        // Then it's an ArithOp?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseArithOp(itr);
                }
        }
        return ret;
}

bool Parser::parseRelation(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        ret = this->parseTerm(itr);
        if (ret){
                if (((*itr)->type == T_OP_REL_GREATER) ||
                    ((*itr)->type == T_OP_REL_LESS) ||
                    ((*itr)->type == T_OP_REL_GREATER_EQUAL) ||
                    ((*itr)->type == T_OP_REL_LESS_EQUAL) ||
                    ((*itr)->type == T_OP_REL_EQUAL) ||
                    ((*itr)->type == T_OP_REL_NOT_EQUAL))
                {
                        debug_print_token(**itr);
                        // Then it's a relation?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseRelation(itr);
                }
        }
        return ret;
}

bool Parser::parseProcedureCall(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else {
                ret = false; // TODO Handle error expected identifier
                return ret;
        }


        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = this->parseArgumentList(itr);
                if ( ret == false){
                        return ret;
                }

                if ((*itr)->type == T_SYM_LPAREN){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        // ret = true; // ret should already be set to true by parseArg
                }else{
                        ret = false;
                        // TODO Handle error Missing bracket
                }
        }
        return ret;
}

bool Parser::parseArgumentList(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;

        // Parse expression
        ret = this->parseExpression(itr);
        if (ret == false){
                return ret;
        }

        // if comma, keep parsing Argument List
        if ((*itr)->type == T_SYM_COMMA){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                //Parse ArgumentList
                ret = this->parseArgumentList(itr);
        }
        return ret;
}

bool Parser::parseTerm(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        ret = this->parseFactor(itr);
        if (ret){
                if (((*itr)->type == T_OP_TERM_DIVIDE) ||
                    ((*itr)->type == T_OP_TERM_MULTIPLY))
                {
                        debug_print_token(**itr);
                        // Then it's a term?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseTerm(itr);
                }
        }
        return ret;
}

bool Parser::parseName(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else {
                ret = false; // TODO Handle error
                return ret;
        }

        if ((*itr)->type == T_SYM_LBRACKET){
                // parse expression
                this->inc_ptr(itr); // Move to next token
                ret = this->parseExpression(itr);
                if ( ret == false){
                        return ret;
                }

                if ((*itr)->type == T_SYM_LBRACKET){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        // ret = true; // ret should already be set to true by parseExpression
                }else{
                        ret = false;
                        // TODO Handle error Missing bracket
                }
        }
        return ret;
}

bool Parser::parseFactor(std::list<token_t>::iterator *itr)
{
        debug_print_call()
        bool ret = false;
        // Check for L_Paren
        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = this->parseExpression(itr);
                if ((*itr)->type == T_SYM_RPAREN){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        // this->parseExpression(itr); // Why is this here?
                }else{
                        ret = false; // TODO Handle Missing parentheses
                }
        }else if ((*itr)->type == T_RW_TRUE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_FALSE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_STRING){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_OP_ARITH_MINUS){
                if ((*itr)->type == T_CONST_INTEGER ||
                    (*itr)->type == T_CONST_FLOAT){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        ret = true;
                } else if (!ret){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseName(itr);
                }
        }else if ((*itr)->type == T_CONST_INTEGER ||
                 (*itr)->type == T_CONST_FLOAT){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if (!ret){
                ret = this->parseName(itr);
        }else{
                ret = this->parseProcedureCall(itr);
        }
        return ret;
}

void Parser::parse(std::list<token_t> token_list)
{
        std::list<token_t>::iterator itr;
        this->itr_end = token_list.end();
        itr = token_list.begin();
        this->parseExpression(&itr);
}