#pragma once
#include "Token.hpp"
#include <list>

class Parser
{
private:
        bool parseExpression(std::list<token_t>::iterator *itr);
        bool parseFactor(std::list<token_t>::iterator *itr);
        bool parseTerm(std::list<token_t>::iterator *itr);

        // bool parseName(std::list<token_t>::iterator *itr);
        // bool parseNumber(std::list<token_t>::iterator *itr);
        // bool parseProcedureCall(std::list<token_t>::iterator *itr);
public:
        Parser();
        ~Parser();
        void parse(std::list<token_t> token_list);
};