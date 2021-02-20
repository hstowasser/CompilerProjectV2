#include "Parser.hpp"
#include "Token.hpp"

#if 1
#define debug_print_token(itr) print_token(itr)
#endif

Parser::Parser(/* args */)
{
}

Parser::~Parser()
{
}

bool Parser::parseExpression(std::list<token_t>::iterator *itr)
{
        // arithOp > relation > term > factor
        return false;
}

bool Parser::parseTerm(std::list<token_t>::iterator *itr) // TODO Check this logic
{
        bool ret = false;
        // std::list<token_t>::iterator itr_keep = *itr;
        ret = this->parseFactor(itr);
        if (ret){
                if (((*itr)->type == T_OP_TERM_DIVIDE) ||
                    ((*itr)->type == T_OP_TERM_MULTIPLY))
                {
                        debug_print_token(**itr);
                        // Then it's a term?
                        (*itr)++; // Move to next token
                        ret = this->parseTerm(itr);
                }
        }
        return ret;
}

bool Parser::parseName(std::list<token_t>::iterator *itr)
{
        bool ret = false;
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
                ret = true;
        }else {
                ret = false; // TODO Handle error
                return ret;
        }

        if ((*itr)->type == T_SYM_LBRACKET){
                // parse expression
                this->parseExpression(itr);

                if ((*itr)->type != T_SYM_LBRACKET){
                        ret = false;
                        // TODO Handle error Missing bracket
                }
        }
        return ret;
}

bool Parser::parseFactor(std::list<token_t>::iterator *itr)
{
        bool ret = false;
        // Check for L_Paren
        if ((*itr)->type == T_SYM_LPAREN){
                ret = this->parseExpression(itr);
                if ((*itr)->type == T_SYM_RPAREN){
                        (*itr)++; // Move to next token
                        debug_print_token(**itr);
                        this->parseExpression(itr);
                }else{
                        ret = false;
                }
        }else if ((*itr)->type == T_RW_TRUE){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_FALSE){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_STRING){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
                ret = true;
        }else if ((*itr)->type == T_OP_ARITH_MINUS){
                if ((*itr)->type == T_CONST_INTEGER ||
                    (*itr)->type == T_CONST_FLOAT){
                        debug_print_token(**itr);
                        (*itr)++; // Move to next token
                        ret = true;
                } else if (!ret){
                        debug_print_token(**itr);
                        (*itr)++; // Move to next token
                        ret = this->parseName(itr);
                }
        }else if ((*itr)->type == T_CONST_INTEGER ||
                 (*itr)->type == T_CONST_FLOAT){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
                ret = true;
        }else if (!ret){
                ret = this->parseName(itr);
        }else{
                // ret = this->parseProcedureCall(itr); TODO
        }
        return ret;
}

void Parser::parse(std::list<token_t> token_list)
{
        std::list<token_t>::iterator itr;
        itr = token_list.begin();
        this->parseTerm(&itr);
}