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

        bool parseDestination(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);

        bool parseProcedureHeader(std::list<token_t>::iterator *itr, bool global);
        bool parseProcedureBody(std::list<token_t>::iterator *itr);

        bool parseParameterList(std::list<token_t>::iterator *itr, bool global = false, symbol_t* symbol = NULL);
        bool parseParameter(std::list<token_t>::iterator *itr, bool global, type_holder_t* parameter_type) {return this->parseVariableDeclaration(itr, global, parameter_type);}

        bool parseProcedureDeclaration(std::list<token_t>::iterator *itr, bool global);
        bool parseVariableDeclaration(std::list<token_t>::iterator *itr, bool global, type_holder_t* parameter_type = NULL);

        bool parseTypeMark(std::list<token_t>::iterator *itr, bool global = false, symbol_t* symbol = NULL);

        // All of these will need to return/check types
        bool parseExpression(std::list<token_t>::iterator *itr, type_holder_t* parameter_type = NULL);
        bool parseRelation(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseArithOp(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseFactor(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseTerm(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseName(std::list<token_t>::iterator *itr, type_holder_t* parameter_type); // TODO how to track array types?
        bool parseProcedureCall(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        
        bool parseArgumentList(std::list<token_t>::iterator *itr);
        
        bool AddSymbol_Helper(std::list<token_t>::iterator *itr, bool global, symbol_t symbol);
        bool FindVariableType_Helper(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool FindSymbol_Helper(std::list<token_t>::iterator *itr, symbol_t* symbol);
        bool GetProcedureType(type_holder_t* parameter_type);
        
        void next_token(std::list<token_t>::iterator *itr);
        std::list<token_t>::iterator itr_end;

        Scope* scope;

public:
        Parser(Scope* scope);
        ~Parser();
        void parse(std::list<token_t> token_list);
};