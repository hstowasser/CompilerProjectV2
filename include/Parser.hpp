#pragma once
#include "Token.hpp"
#include "Scope.hpp"
#include <list>
#include <map>

typedef struct {
        unsigned int reg_i;
        unsigned int start_label;
        // unsigned int inc_label;
        unsigned int end_label;
        type_holder_t ret_arr_type;
} array_op_params;

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
        bool parseParameter(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);

        bool parseProcedureDeclaration(std::list<token_t>::iterator *itr, bool global);
        bool parseVariableDeclaration(std::list<token_t>::iterator *itr, bool global, type_holder_t* parameter_type = NULL);

        bool parseTypeMark(std::list<token_t>::iterator *itr, bool global = false, symbol_t* symbol = NULL);

        // All of these will need to return/check types
        bool parseExpression(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseRelation(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseArithOp(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseFactor(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseTerm(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        bool parseName(std::list<token_t>::iterator *itr, type_holder_t* parameter_type); // TODO how to track array types?
        bool parseProcedureCall(std::list<token_t>::iterator *itr, type_holder_t* parameter_type);
        
        bool parseArgumentList(std::list<token_t>::iterator *itr, symbol_t procedure_symbol, std::list<unsigned int> *regs);

        bool AddSymbol_Helper(std::list<token_t>::iterator *itr, bool global, symbol_t symbol);
        bool FindVariableType_Helper(std::list<token_t>::iterator *itr, type_holder_t* parameter_type, bool* global = NULL);
        bool FindSymbol_Helper(std::list<token_t>::iterator *itr, symbol_t* symbol);
        bool GetProcedureType(type_holder_t* parameter_type);
        
        void next_token(std::list<token_t>::iterator *itr);
        std::list<token_t>::iterator itr_end;

        Scope* scope;

        // CODE GENERATION
        unsigned int genAlloca(token_type_e type, bool is_arr = false, unsigned int len = 0);
        unsigned int genLoadReg(token_type_e type, unsigned int location_reg, bool global = false); // %d = load <type>, <type>* %location_reg
        void genStoreReg(token_type_e type, unsigned int expr_reg, unsigned int dest_reg, bool global = false); // store <type> %expression, <type>* %destination
        void genStoreConst(unsigned int dest_reg, bool value); // store i8 value, i8* %destination
        void genStoreConst(unsigned int dest_reg, int value); // store i32 value, i32* %destination
        void genStoreConst(unsigned int dest_reg, float value); // store float value, float* %destination

        void genVariableDeclaration(symbol_t* symbol, bool global);
        void genAssignmentStatement(type_holder_t dest_type, type_holder_t expr_type);
        void genProgramHeader();
        void genProgramBodyEnd();
        void genProcedureHeader(symbol_t symbol, std::string name);
        void genProcedureEnd();
        unsigned int genProcedureCall(symbol_t symbol, std::string name, std::list<unsigned int> regs);
        void genArgumentsList();
        void genConstant(std::list<token_t>::iterator itr, type_holder_t* parameter_type, bool is_negative = false); // used in parseFactor
        void genReturn(token_type_e type, unsigned int reg);

        unsigned int genIntToLong(unsigned int reg);
        unsigned int genIntToFloat(unsigned int reg);
        unsigned int genFloatToInt(unsigned int reg);
        unsigned int genBoolToInt(unsigned int reg);
        unsigned int genIntToBool(unsigned int reg);
        unsigned int genGEP(type_holder_t parameter_type, unsigned int index_reg, bool global = false);
        unsigned int genGEP_Head(type_holder_t parameter_type,  bool global /*= false*/);
        unsigned int genTerm(token_type_e op, token_type_e type, unsigned int reg_a, unsigned int reg_b);
        unsigned int genRelation(token_type_e op, token_type_e type_a, unsigned int reg_a, token_type_e type_b, unsigned int reg_b);
        unsigned int genRelationStrings(token_type_e op, unsigned int reg_a, unsigned int reg_b);
        unsigned int genArithOp(token_type_e op, token_type_e type_a, unsigned int reg_a, token_type_e type_b, unsigned int reg_b);
        unsigned int genExpression(token_type_e op, unsigned int reg_a, unsigned int reg_b);
        unsigned int genNegate(token_type_e type, unsigned int reg);

        void genIfHead(unsigned int reg, unsigned int if_label, unsigned int else_label);
        void genIfElse(unsigned int else_label, unsigned int end_label);
        void genIfEnd(unsigned int end_label);

        void genLoopHead(unsigned int start_label);
        void genLoopCondition(unsigned int reg, unsigned int body_label, unsigned int end_label, bool dont_tuncate = false);
        void genLoopEnd( unsigned int start_label, unsigned int end_label);

        std::list<std::tuple<std::string, symbol_t>> paramSymbolBuffer;

        array_op_params genSetupArrayOp(type_holder_t* type_a, type_holder_t* type_b, token_type_e result_type); // Changes type_a/b is_arr=false _is_global=false reg_ct to value at index

        type_holder_t genEndArrayOp(array_op_params params, unsigned int expr_reg); // returns resultant array params.ret_arr_type

public:
        Parser(Scope* scope);
        ~Parser();
        bool parse(std::list<token_t> token_list);

        std::string program_name;
};