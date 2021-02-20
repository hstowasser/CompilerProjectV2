#pragma once
#include "Token.hpp"
#include <list>

class Parser
{
private:
        bool parseProgram(std::list<token_t>::iterator *itr); // TODO Incomplete needs parseProgramBody
        bool parseProgramHeader(std::list<token_t>::iterator *itr);
        bool parseProgramBody(std::list<token_t>::iterator *itr); // TODO needs parseDelcaration and parseStatement
        bool parseDeclaration(std::list<token_t>::iterator *itr); // TODO needs prodecure/variable/typeDeclaration 
        bool parseStatement(std::list<token_t>::iterator *itr); // TODO lots to do

        bool parseProcedureDeclaration(std::list<token_t>::iterator *itr); //TODO needs TypeMark and ParameterList
        bool parseVariableDeclaration(std::list<token_t>::iterator *itr); //TODO needs TypeMark and Bound
        bool parseTypeDeclaration(std::list<token_t>::iterator *itr);

        bool parseTypeMark(std::list<token_t>::iterator *itr);

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