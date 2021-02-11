#include "Parser.hpp"
#include "Token.hpp"


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
        print_token(**itr);
        if (ret){
                if (((*itr)->type == T_OP_TERM_DIVIDE) ||
                    ((*itr)->type == T_OP_TERM_MULTIPLY))
                {
                        print_token(**itr);
                        // Then it's a term?
                        (*itr)++; // Move to next token
                        ret = this->parseTerm(itr);
                }
        }
        return ret;
}

bool Parser::parseFactor(std::list<token_t>::iterator *itr)
{
        bool ret = false;
        // Check for L_Paren
        if ((*itr)->type == T_SYM_LPAREN){
                (*itr)++; // Move to next token
                ret = this->parseExpression(itr);
                if ((*itr)->type == T_SYM_RPAREN){
                        this->parseExpression(itr);
                }else{
                        ret = false;
                }
        }else if ((*itr)->type == T_RW_TRUE){
                (*itr)++; // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_FALSE){
                (*itr)++; // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_STRING){
                (*itr)++; // Move to next token
                ret = true;
        }else if ((*itr)->type == T_OP_ARITH_MINUS){
                (*itr)++; // Move to next token
                // ret = this->parseNumber(itr); TODO
                if (!ret){
                        // ret = this->parseName(itr); TODO
                }
        }else{
                // ret = this->parseProcedureCall(itr); TODO
        }
        return ret;
}

void Parser::parse(std::list<token_t> token_list)
{

}