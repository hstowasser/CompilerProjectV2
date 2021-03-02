#pragma once
#include "Token.hpp"
#include "Scope.hpp"
#include <list>
#include <map>

class Parser
{
private:
        bool parseProgram(std::list<token_t>::iterator *itr);
        bool parseProgramHeader(std::list<token_t>::iterator *itr);
        bool parseProgramBody(std::list<token_t>::iterator *itr);
        bool parseDeclaration(std::list<token_t>::iterator *itr);
        bool parseStatement(std::list<token_t>::iterator *itr);

        bool parseAssignmentStatement(std::list<token_t>::iterator *itr);
        bool parseIfStatement(std::list<token_t>::iterator *itr);
        bool parseLoopStatement(std::list<token_t>::iterator *itr);
        bool parseReturnStatement(std::list<token_t>::iterator *itr);

        bool parseDestination(std::list<token_t>::iterator *itr);

        bool parseProcedureHeader(std::list<token_t>::iterator *itr, bool global);
        bool parseProcedureBody(std::list<token_t>::iterator *itr);

        bool parseParameterList(std::list<token_t>::iterator *itr, symbol_t* symbol = NULL);
        bool parseParameter(std::list<token_t>::iterator *itr, type_holder_t* parameter_type) {return this->parseVariableDeclaration(itr, false, parameter_type);}

        bool parseProcedureDeclaration(std::list<token_t>::iterator *itr, bool global);
        bool parseVariableDeclaration(std::list<token_t>::iterator *itr, bool global, type_holder_t* parameter_type = NULL);
        bool parseTypeDeclaration(std::list<token_t>::iterator *itr, bool global);

        bool parseTypeMark(std::list<token_t>::iterator *itr, bool global = false, symbol_t* symbol = NULL);

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

        Scope* scope;

public:
        Parser(Scope* scope);
        ~Parser();
        void parse(std::list<token_t> token_list);
};