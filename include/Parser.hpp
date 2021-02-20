#pragma once
#include "Token.hpp"
#include <list>

class Parser
{
private:
        bool parseExpression(std::list<token_t>::iterator *itr);
        bool parseFactor(std::list<token_t>::iterator *itr);
        bool parseTerm(std::list<token_t>::iterator *itr);
        bool parseName(std::list<token_t>::iterator *itr);
        bool parseProcedureCall(std::list<token_t>::iterator *itr);
        bool parseArgumentList(std::list<token_t>::iterator *itr);
        bool parseArithOp(std::list<token_t>::iterator *itr);
        bool parseRelation(std::list<token_t>::iterator *itr);

        
        void inc_ptr(std::list<token_t>::iterator *itr);
        std::list<token_t>::iterator itr_end;
public:
        Parser();
        ~Parser();
        void parse(std::list<token_t> token_list);
};