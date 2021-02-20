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
        // TODO implement
        return false;
}

bool Parser::parseArithOp(std::list<token_t>::iterator *itr)
{
        bool ret = false;
        ret = this->parseRelation(itr);
        if (ret){
                if (((*itr)->type == T_OP_ARITH_MINUS) ||
                    ((*itr)->type == T_OP_ARITH_PLUS))
                {
                        debug_print_token(**itr);
                        // Then it's an ArithOp?
                        (*itr)++; // Move to next token
                        ret = this->parseArithOp(itr);
                }
        }
        return ret;
}

bool Parser::parseRelation(std::list<token_t>::iterator *itr)
{
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
                        (*itr)++; // Move to next token
                        ret = this->parseRelation(itr);
                }
        }
        return ret;
}

bool Parser::parseProcedureCall(std::list<token_t>::iterator *itr)
{
        bool ret = false;
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
        }else {
                ret = false; // TODO Handle error expected identifier
                return ret;
        }


        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                (*itr)++; // Move to next token
                ret = this->parseArgumentList(itr);
                if ( ret == false){
                        return ret;
                }

                if ((*itr)->type == T_SYM_LPAREN){
                        debug_print_token(**itr);
                        (*itr)++; // Move to next token
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
        bool ret = false;

        // Parse expression
        ret = this->parseExpression(itr);
        if (ret == false){
                return ret;
        }

        // if comma, keep parsing Argument List
        if ((*itr)->type == T_SYM_COMMA){
                debug_print_token(**itr);
                (*itr)++; // Move to next token

                //Parse ArgumentList
                ret = this->parseArgumentList(itr);
        }
        return ret;
}

bool Parser::parseTerm(std::list<token_t>::iterator *itr)
{
        bool ret = false;
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
                (*itr)++; // Move to next token
                ret = this->parseExpression(itr);
                if ( ret == false){
                        return ret;
                }

                if ((*itr)->type == T_SYM_LBRACKET){
                        debug_print_token(**itr);
                        (*itr)++; // Move to next token
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