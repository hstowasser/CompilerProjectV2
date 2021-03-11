#include "Parser.hpp"
#include "Token.hpp"

#include <stdio.h>
#include <stdarg.h>

#if 1
#define debug_print_call() printf("%s\n", __FUNCTION__)
#else
#define debug_print_call()
#endif

#if 1
#define debug_print_token(itr) print_token(itr)
#else
#define debug_print_token(itr)
#endif



#define error_printf(token, fmt, ...) printf("ERROR: Line %d - " fmt, (token)->line_num, ##__VA_ARGS__)

#define get_c_string(itr) (*((*itr)->getStringValue())).c_str() // Gets token identifier as c string
#define get_string(itr) *((*itr)->getStringValue()) // Gets token identifier as std::string


Parser::Parser(Scope* scope)
{
        this->scope = scope;
}

Parser::~Parser()
{
}

void Parser::next_token(std::list<token_t>::iterator *itr)
{
        debug_print_token(**itr);
        if (*itr != this->itr_end) {
                (*itr)++;
        }else{
                error_printf( *itr, "Unknown error,  \n");
        }
}

bool Parser::AddSymbol_Helper(std::list<token_t>::iterator *itr, bool global, symbol_t symbol)
{
        // Check if symbol already exists
        bool success;
        if (global){
                this->scope->FindGlobal(get_string(itr), &success);
        }else{
                this->scope->FindLocal(get_string(itr), &success);
        }

        if( success == true){
                // Error Symbol already exists
                error_printf( *itr, "%s redefined \n", get_c_string(itr));
                return false;
        }

        if (global){
                this->scope->AddGlobalSymbol(*(*itr)->getStringValue(), symbol);
        }else{
                this->scope->AddSymbol(*(*itr)->getStringValue(), symbol);
        }
        return true;
}

bool Parser::FindVariableType_Helper(std::list<token_t>::iterator *itr, type_holder_t* parameter_type, bool* global /*= NULL*/)
{
        bool success;
        bool global_hold = false;
        std::map<std::string,symbol_t>::iterator temp;
        temp = this->scope->FindLocal(get_string(itr), &success);
        if (success){
                *parameter_type = temp->second.variable_type;

        } else {
                temp = this->scope->FindGlobal(get_string(itr), &success);
                if (success) {
                        *parameter_type = temp->second.variable_type;
                        global_hold = true;
                } else {
                        error_printf( *itr, "Symbol %s is not defined \n", get_c_string(itr));
                }
        }
        if (global != NULL){
                *global = global_hold;
        }
        return success;
}

bool Parser::FindSymbol_Helper(std::list<token_t>::iterator *itr, symbol_t* symbol)
{
    bool success;
    std::map<std::string,symbol_t>::iterator temp;
    temp = this->scope->Find(get_string(itr), &success);
    if (success){
            *symbol = temp->second;
    } else {
            error_printf( *itr, "Symbol %s is not defined \n", get_c_string(itr));
    }
    return success;
}

bool Parser::GetProcedureType(type_holder_t* parameter_type)
{
        std::string procedure_name = this->scope->getProcedureName();
        bool success;
        std::map<std::string,symbol_t>::iterator temp;

        temp = this->scope->Find(procedure_name, &success);
        if (success){
                *parameter_type = temp->second.variable_type;
        } else {
                // TODO verify that return statements are not valid in Program scope
                printf( "Failed to obtain Procedure return type. \n");
        }
        return success;
}

bool Parser::parseProgram(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        // Parse program header
        ret = this->parseProgramHeader(itr);

        // Parse program body
        ret = this->parseProgramBody(itr);

        // check for period
        if ((*itr)->type == T_SYM_PERIOD){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected Period \n");
                return false;
        }

        return ret;
}

bool Parser::parseProgramBody(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        // parse declarations
        while ( this->parseDeclaration(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected Semicolon \n");
                        return false;
                }
        }

        // check for "begin"
        if ((*itr)->type == T_RW_BEGIN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"begin\" in program body \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected Semicolon \n");
                        return false;
                }
        }

        // check for "end" "program"
        if ((*itr)->type == T_RW_END){
                this->next_token(itr); // Move to next token

                if ((*itr)->type == T_RW_PROGRAM){
                        genProgramBodyEnd();
                        this->next_token(itr); // Move to next token
                        ret = true; // TODO Check that this is correct
                }else{
                        error_printf( *itr, "Expected \"end program\" \n");
                        return false;
                }
        }else{
                error_printf( *itr, "Expected \"end program\" \n");
                return false;
        }

        return ret;
}

bool Parser::parseDestination(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        bool global;

        // check identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Look up in symbol table and return type
                ret = this->FindVariableType_Helper(itr, parameter_type, &global);
                if( !ret){
                        return false;
                }

                this->next_token(itr); // Move to next token
        }else{
                return false; // Not a valid destination
        }


        // if open bracket
        if ((*itr)->type == T_SYM_LBRACKET){
        
                if (parameter_type->is_array == false){
                        error_printf( *itr, "Identifier is not an array\n");
                        return false;
                } else {
                        // Set is_array to false because we are grabbing one element
                        parameter_type->is_array = false;
                }

                this->next_token(itr); // Move to next token

                type_holder_t expr_type;

                // then parse expression
                ret = this->parseExpression(itr, &expr_type);
                if ( !ret ){
                        return false;
                }

                if ( expr_type.type != T_RW_INTEGER ){
                        error_printf( *itr, "Array index is invalid type \n");
                        return false;
                }

                // CODEGEN GEP then update parameter_type->reg_ct
                unsigned int index_reg = this->genIntToLong(expr_type.reg_ct);
                parameter_type->reg_ct = this->genGEP(*parameter_type, index_reg, global);
                parameter_type->is_array = false; // We are looking at only a single element
                parameter_type->_is_global = false; // It has been imported

                // check close bracket
                if ((*itr)->type == T_SYM_RBRACKET){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing bracket \n");
                        return false;
                }
        } else if (parameter_type->is_array){
                // GEP
                parameter_type->reg_ct = this->genGEP_Head(*parameter_type, global);
        }
        

        return ret;
}

bool Parser::parseAssignmentStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        type_holder_t dest_type;
        type_holder_t expr_type;

        ret = this->parseDestination(itr, &dest_type);
        if (!ret){
                return false; // Not an assignment statement
        }

        // check for assignment op :=
        if ((*itr)->type == T_OP_ASIGN_EQUALS){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \":=\" \n");
                return false;
        }        

        // parse expression
        ret = this->parseExpression(itr, &expr_type); 
        if (!ret){
                return false;
        }

        // Check that destination type matches expression type
        if (type_holder_cmp(dest_type, expr_type)){
                // Both are the same
                
        } else if ( ((expr_type.type == T_RW_INTEGER) || (expr_type.type == T_RW_FLOAT)) &&
                ((dest_type.type == T_RW_INTEGER) || (dest_type.type == T_RW_FLOAT))) {
                // Combinations of int and float are allowed
                // Typecase to destination
                if ( dest_type.type == T_RW_INTEGER){
                        expr_type.reg_ct = this->genFloatToInt(expr_type.reg_ct);
                        expr_type.type = T_RW_INTEGER;
                } else {
                        expr_type.reg_ct = this->genIntToFloat(expr_type.reg_ct);
                        expr_type.type = T_RW_FLOAT;
                }
        } else if ( ((expr_type.type == T_RW_INTEGER) || (expr_type.type == T_RW_BOOL)) &&
                ((dest_type.type == T_RW_INTEGER) || (dest_type.type == T_RW_BOOL))) {
                // Combinations of int and bool are allowed
                // Typecast to destination
                if ( dest_type.type == T_RW_INTEGER){
                        expr_type.reg_ct = this->genBoolToInt(expr_type.reg_ct);
                        expr_type.type = T_RW_INTEGER;
                } else {
                        expr_type.reg_ct = this->genIntToBool(expr_type.reg_ct);
                        expr_type.type = T_RW_BOOL;
                }
        } else {
                error_printf( *itr, "Destination type does not match expression \n"); // TODO print types
                return false;
        }

        // Codegeneration
        // if int | float | bool
                // store <destination_type> <expression>, <destination_type>* <destination_ptr>
                // store <i32/float/bool> %current, <i32/float/bool>* symbol_table_lookup_%reg
        // if string
                // change pointer
        // if array
                // use llvm.memcpy

        genAssignmentStatement(dest_type, expr_type);

        return ret; 
}

bool Parser::parseIfStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        type_holder_t expr_type;
        unsigned int if_label;
        unsigned int else_label;
        unsigned int end_label;

        if ((*itr)->type == T_RW_IF){
                this->next_token(itr); // Move to next token
        }else{
                return false; //Not an if statement
        }

        if ((*itr)->type == T_SYM_LPAREN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"(\" after IF \n");
                return false;
        }

        ret = this->parseExpression(itr, &expr_type);
        if (!ret){
                return false;
        }

        // Check that expression type is either int or bool
        if ( !(expr_type.type == T_RW_BOOL ||
               expr_type.type == T_RW_INTEGER)){
                error_printf(*itr, "IF Expression must be bool or int \n");
                return false;
        }

        // TODO generate code
        if (expr_type.type == T_RW_INTEGER){
                expr_type.reg_ct = this->genIntToBool(expr_type.reg_ct); // Convert to bool
        }
        if_label = this->scope->NewLabel();
        else_label = this->scope->NewLabel();
        end_label = this->scope->NewLabel();

        this->genIfHead( expr_type.reg_ct, if_label, else_label);

        if ((*itr)->type == T_SYM_RPAREN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \")\" before THEN \n");
                return false;
        }

        if ((*itr)->type == T_RW_THEN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected THEN \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }

        this->genIfElse(else_label, end_label);

        // if "else"
        if ((*itr)->type == T_RW_ELSE){
                this->next_token(itr); // Move to next token

                // parse statements
                while ( this->parseStatement(itr) ){
                        // check for ;
                        if ((*itr)->type == T_SYM_SEMICOLON){
                                this->next_token(itr); // Move to next token
                        }else{
                                error_printf( *itr, "Expected semicolon \n");
                                return false;
                        }
                }
        }

        this->genIfEnd(end_label);

        if ((*itr)->type == T_RW_END){
                this->next_token(itr); // Move to next token

                if ((*itr)->type == T_RW_IF){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected \"END IF\" \n");
                        return false;
                }
        }else{
                error_printf( *itr, "Expected \"END IF\" \n");
                return false;
        }

        return ret;
}

bool Parser::parseLoopStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        type_holder_t expr_type;

        if ((*itr)->type == T_RW_FOR){
                this->next_token(itr); // Move to next token
        }else{
                return false; //Not a for statement
        }

        if ((*itr)->type == T_SYM_LPAREN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"(\" after FOR \n");
                return false;
        }

        ret = this->parseAssignmentStatement(itr);
        if (!ret){
                return false;
        }

        if ((*itr)->type == T_SYM_SEMICOLON){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected semicolon \n");
                return false;
        }

        ret = this->parseExpression(itr, &expr_type);
        if (!ret){
                return false;
        }

        // Check that expression type is either int or bool
        if ( !(expr_type.type == T_RW_BOOL ||
               expr_type.type == T_RW_INTEGER)){
                error_printf(*itr, "IF Expression must be bool or int \n");
                return false;
        }

        if ((*itr)->type == T_SYM_RPAREN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \")\" \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }


        if ((*itr)->type == T_RW_END){
                this->next_token(itr); // Move to next token

                if ((*itr)->type == T_RW_FOR){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected \"END FOR\" \n");
                        return false;
                }
        }else{
                error_printf( *itr, "Expected \"END FOR\" \n");
                return false;
        }

        return ret;
}

bool Parser::parseReturnStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        type_holder_t expr_type;
        type_holder_t procedure_type;

        if ((*itr)->type == T_RW_RETURN){
                this->next_token(itr); // Move to next token
        }else{
                return false; // Not a return statement
        }

        ret = this->parseExpression(itr, &expr_type);
        if (!ret){
                return false;
        }

        //Check that procedure type exactly matches return type
        ret = GetProcedureType(&procedure_type);
        if (!ret){
                return false; // Failed to find procedure type
        }

        if(expr_type.type != procedure_type.type){
                error_printf( *itr, "Procedure type and return type do not match \n"); // TODO print types
                return false;
        }

        genReturn(procedure_type.type, expr_type.reg_ct);

        return ret;
}

bool Parser::parseStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        if ((ret = this->parseAssignmentStatement(itr))){

        }else if ((ret = this->parseIfStatement(itr))){

        }else if ((ret = this->parseLoopStatement(itr))){

        }else if ((ret = this->parseReturnStatement(itr))){

        } // else no valid statement

        return ret;
}

bool Parser::parseDeclaration(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        bool global = false;

        if ((*itr)->type == T_RW_GLOBAL){
                this->next_token(itr); // Move to next token
                global = true;
        }

        if ((ret = this->parseProcedureDeclaration(itr, global))){
                // Procedure Declaration
        }else if ((ret = this->parseVariableDeclaration(itr, global))){
                // Variable Declaration
        }//else if ((ret = this->parseTypeDeclaration(itr, global))){
        //        // Type Declaration
        //} // else no valid declaration

        return ret;
}

bool Parser::parseProcedureBody(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        // parse declarations
        while ( this->parseDeclaration(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }

        // check for "begin"
        if ((*itr)->type == T_RW_BEGIN){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"BEGIN\" in procedure body \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }

        // check for "end" "procedure"
        if ((*itr)->type == T_RW_END){
                this->next_token(itr); // Move to next token

                if ((*itr)->type == T_RW_PROCEDURE){
                        this->next_token(itr); // Move to next token
                        ret = true; // TODO Check that this is correct
                }else{
                        error_printf( *itr, "Expected \"END PROCEDURE\" \n");
                        return false;
                }
        }else{
                error_printf( *itr, "Expected \"END PROCEDURE\" \n");
                return false;
        }


        return ret;
}

bool Parser::parseProcedureDeclaration(std::list<token_t>::iterator *itr, bool global)
{
        debug_print_call();
        bool ret = false;

        // parseProcedureHeader
        ret = this->parseProcedureHeader(itr, global);
        if (!ret){
                return false;
        }

        ret = this->parseProcedureBody(itr);

        genProcedureEnd();
        this->scope->PopScope();

        return ret;
}

bool Parser::parseProcedureHeader(std::list<token_t>::iterator *itr, bool global)
{
        symbol_t symbol;
        std::string name;
        debug_print_call();
        bool ret = false;

        // check for "procedure"
        if ((*itr)->type == T_RW_PROCEDURE){
                this->next_token(itr); // Move to next token
        }else{
                return false; // Not a procedure header
        }

        // check identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol.type = ST_PROCEDURE;
                name = *(*itr)->getStringValue();

                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"PROCEDURE\" \n");
                return false;
        }

        // check colon
        if ((*itr)->type == T_SYM_COLON){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected colon \n");
                return false;
        }

        // parseTypeMark
        ret = this->parseTypeMark(itr,global,&symbol);
        if (!ret){
                return false;
        }

        // check lparen, parseParameterList, check rparen
        if ((*itr)->type == T_SYM_LPAREN){
                this->next_token(itr); // Move to next token

                ret = this->parseParameterList(itr, global, &symbol);

                if ((*itr)->type == T_SYM_RPAREN){
                        this->next_token(itr); // Move to next token
                } else {
                        error_printf( *itr, "Expected closing parentheses after procedure parameters \n");
                        return false;
                }
        }else{
                error_printf( *itr, "Expected parentheses after procedure identifier \n");
                return false;
        }

        
        if (global){
                symbol._procedure_ct = 0;
                this->scope->AddGlobalSymbol(name, symbol);
                this->scope->PushScope(name);
        }else{
                symbol._procedure_ct = this->scope->procedure_ct; // Used for code generation
                this->scope->procedure_ct++;
                // To allow for recursive calls add procedure symbol to both current and next scope
                this->scope->AddSymbol(name, symbol);
                this->scope->PushScope(name);
                this->scope->AddSymbol(name, symbol);
        }

        auto it = this->paramSymbolBuffer.end();
        while(it != this->paramSymbolBuffer.begin()) {
                auto tup = this->paramSymbolBuffer.back();
                std::string name = std::get<0>(tup);
                symbol_t symbol = std::get<1>(tup);
                symbol.variable_type.reg_ct = scope->reg_ct_local++; // Set the reg_ct for it.
                this->scope->AddSymbol(name, symbol);
                this->paramSymbolBuffer.pop_back();
                it = this->paramSymbolBuffer.end();
        }
                

        genProcedureHeader(symbol, name);

        return ret;
}

bool Parser::parseParameterList(std::list<token_t>::iterator *itr, bool global /*= false*/, symbol_t* symbol /*= NULL*/)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_parameter_type;
        std::list<type_holder_t> temp_param_list;
        symbol->parameter_ct = 0;

        bool first = true;
        do{
                if (!first){
                        // Skip comma
                        this->next_token(itr); // Move to next token
                }
                ret = this->parseParameter(itr, &temp_parameter_type); // Parameters of a global function are local
                temp_param_list.push_back(temp_parameter_type);
                symbol->parameter_ct++; // Increment parameterlist counter
                first = false;
        } while(ret && (*itr)->type == T_SYM_COMMA);

        // Allocate memory for parameter_type_arr
        symbol->parameter_type_arr = (type_holder_t*)calloc(symbol->parameter_ct, sizeof(type_holder_t));
        if (symbol->parameter_type_arr == NULL){
                // Failed to allocate memory... Panic
                printf("Failed to allocate memory \n");
                return false;
        }

        // Copy temp_param_list into symbol->parameter_type_arr
        std::list<type_holder_t>::iterator it;
        unsigned int i = 0;
        for ( it = temp_param_list.begin(); it != temp_param_list.end(); it++){
                symbol->parameter_type_arr[i] = *it;
                i++;
        }

        return ret;
}

/**
 *      symbol is only used by parseParameterList.
 */
bool Parser::parseVariableDeclaration(std::list<token_t>::iterator *itr, bool global /*= false*/, type_holder_t* parameter_type /*= NULL*/)
{
        debug_print_call();
        bool ret = false;
        std::string name;
        symbol_t symbol;

        // check for "variable"
        if ((*itr)->type == T_RW_VARIABLE){
                this->next_token(itr); // Move to next token
        }else{
                return false; // Not a variable declaration
        }

        // check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol.type = ST_VARIABLE;
                name = *(*itr)->getStringValue();

                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"VARIABLE\" \n");
                return false;
        }

        // check for colon
        if ((*itr)->type == T_SYM_COLON){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected colon in variable declaration \n");
                return false;
        }

        // parse for type_mark
        ret = this->parseTypeMark(itr, global, &symbol);
        if (!ret){
                return false;
        }
        if ( parameter_type != NULL){
                // If this was called by parseParameterList
                // return type so it can be stored in the procedure symbol
                *parameter_type = symbol.variable_type;
        }

        // check for open bracket
        if ((*itr)->type == T_SYM_LBRACKET){
                this->next_token(itr); // Move to next token

                // parse bound
                if ((*itr)->type == T_CONST_INTEGER){
                        symbol.variable_type.is_array = true;
                        symbol.variable_type.array_length = (unsigned int)(*itr)->getIntValue();
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected bound inside of [] \n");
                        return false;
                }

                // check for close bracket
                if ((*itr)->type == T_SYM_RBRACKET){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing bracket \n");
                        return false;
                }
        }

        genVariableDeclaration( &symbol, global); // Generate code

        if (global){
                this->scope->AddGlobalSymbol(name, symbol);
        }else{
                this->scope->AddSymbol(name, symbol);
        }

        return ret;
}

bool Parser::parseParameter(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        std::string name;
        symbol_t symbol;

        // check for "variable"
        if ((*itr)->type == T_RW_VARIABLE){
                this->next_token(itr); // Move to next token
        }else{
                return false; // Not a variable declaration
        }

        // check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol.type = ST_VARIABLE;
                name = *(*itr)->getStringValue();

                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"VARIABLE\" \n");
                return false;
        }

        // check for colon
        if ((*itr)->type == T_SYM_COLON){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected colon in variable declaration \n");
                return false;
        }

        // parse for type_mark
        ret = this->parseTypeMark(itr, false, &symbol);
        if (!ret){
                return false;
        }
        

        // check for open bracket
        if ((*itr)->type == T_SYM_LBRACKET){
                this->next_token(itr); // Move to next token

                // parse bound
                if ((*itr)->type == T_CONST_INTEGER){
                        symbol.variable_type.is_array = true;
                        symbol.variable_type.array_length = (unsigned int)(*itr)->getIntValue();
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected bound inside of [] \n");
                        return false;
                }

                // check for close bracket
                if ((*itr)->type == T_SYM_RBRACKET){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing bracket \n");
                        return false;
                }
        }

        // genVariableDeclaration( &symbol, global); // Generate code

        symbol.variable_type.is_parameter = true;

        // if (global){
        //         this->scope->AddGlobalSymbol(name, symbol);
        // }else{
        //         this->scope->AddSymbol(name, symbol);
        // }
        this->paramSymbolBuffer.push_back(std::make_tuple(name, symbol));

        if ( parameter_type != NULL){
                // If this was called by parseParameterList
                // return type so it can be stored in the procedure symbol
                *parameter_type = symbol.variable_type;
        }

        return ret;
}

/**     This gets  called by parseProcedureHeader, parseVariableDeclaration and parseTypeMark
 *      if it is called by parseTypeMark4
 * 
 */
bool Parser::parseTypeMark(std::list<token_t>::iterator *itr, bool global, symbol_t* symbol /*= NULL*/)
{
        debug_print_call();
        bool ret = false;

        // check for integer or float or string or bool
        // then get identifier
        if (((*itr)->type == T_RW_INTEGER) ||
            ((*itr)->type == T_RW_FLOAT) ||
            ((*itr)->type == T_RW_STRING) ||
            ((*itr)->type == T_RW_BOOL))
        {
                if (symbol != NULL){
                        symbol->variable_type.type = (*itr)->type;
                }
                this->next_token(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_IDENTIFIER){
                error_printf( *itr, "Symbol %s is not a type\n", get_c_string(itr) );
                this->next_token(itr); // Move to next token
                ret = false;
        }else {
                return false;
        }

        return ret;
}

bool Parser::parseProgramHeader(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        // check for program tag
        if ((*itr)->type == T_RW_PROGRAM){
                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Code must start with \"PROGRAM\" definition\n");
                return false;
        }

        // parse identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Create scope for program
                this->scope->PushScope(*(*itr)->getStringValue());
                this->genProgramHeader();

                this->next_token(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"PROGRAM\" \n");
                return false;
        }

        // check for "is"
        if ((*itr)->type == T_RW_IS){
                this->next_token(itr); // Move to next token
                ret = true;
        }else{
                error_printf( *itr, "Expected \"IS\" after program identifier \n");
                return false;
        }
        return ret;
}

/* NOTES FOR TYPECHECKING
   When doing type checking it would be useful for parseExpression
   to return the type of the expression. This could be difficult to implement
   because expressions may mix types which makes things more complicated.

   Booleans may be converted to ints and vice versa in relational operations.

        Expressions are strongly typed and types
        must match. However there is automatic
        conversion in the arithmetic operators to
        allow any mixing between integers and
        floats.

        "Furthermore, the relational
        operators can compare booleans with
        integers (booleans are converted to integers
        as: false → 0, true → 1; integers are
        converted to bools as: the integer value 0 is
        converted to false, all other integer values
        are converted to true)"

*/
bool Parser::parseExpression(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_arithop;
        type_holder_t temp_expression;
        bool bitwise_op = false;
        token_type_e op;

        // check for not
        if ((*itr)->type == T_RW_NOT){
                this->next_token(itr); // Move to next token
                bitwise_op = true;
        }
        
        ret = this->parseArithOp(itr, &temp_arithop);
        if (!ret) {
                return false;
        }
        if (bitwise_op){
                if (temp_arithop.type == T_RW_INTEGER) {
                        // Good
                } else {
                        error_printf( *itr, "Bitwise operation NOT is only defined for integers \n");
                        return false;
                }
        }

        if (((*itr)->type == T_OP_BITW_AND) ||
            ((*itr)->type == T_OP_BITW_OR)) {
                op = (*itr)->type;

                this->next_token(itr); // Move to next token
                ret = this->parseExpression(itr, &temp_expression);

                // Bitwise operations are only valid for integers
                if ((temp_expression.type == T_RW_INTEGER) &&
                   ((temp_arithop.type == T_RW_INTEGER))) {
                        *parameter_type = temp_arithop;
                        parameter_type->reg_ct = this->genExpression(op, temp_arithop.reg_ct, temp_expression.reg_ct);
                } else {
                        error_printf( *itr, "Bitwise operations AND/OR are only defined for integers \n");
                        return false;
                }
                
        } else {
                if (parameter_type != NULL) {
                        *parameter_type = temp_arithop;
                }                        
        }

        return ret;
}

bool Parser::parseArithOp(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_relation;
        type_holder_t temp_arithop;
        token_type_e op;

        ret = this->parseRelation(itr, &temp_relation); // TODO test type checking
        if (ret){
                if (((*itr)->type == T_OP_ARITH_MINUS) ||
                    ((*itr)->type == T_OP_ARITH_PLUS))
                {
                        op = (*itr)->type;
                        // Then it's an ArithOp?
                        this->next_token(itr); // Move to next token
                        ret = this->parseArithOp(itr, &temp_arithop);

                        if ( ((temp_relation.type == T_RW_INTEGER) || (temp_relation.type == T_RW_FLOAT)) &&
                             ((temp_arithop.type == T_RW_INTEGER) || (temp_arithop.type == T_RW_FLOAT))) {
                                // Combinations of int and float are allowed
                                if (temp_relation.type == T_RW_FLOAT || temp_arithop.type == T_RW_FLOAT){
                                        parameter_type->type = T_RW_FLOAT; // Default to float
                                }else{
                                        parameter_type->type = T_RW_INTEGER;
                                }
                                parameter_type->reg_ct =
                                        this->genArithOp(op, temp_relation.type, temp_relation.reg_ct, temp_arithop.type, temp_arithop.reg_ct);
                        } else {
                                error_printf( *itr, "Arithmetic Ops (+,-) are only defined for integers and floats \n");
                                return false;
                        }
                }else{
                        *parameter_type = temp_relation;
                }
        }        

        return ret;
}

bool Parser::parseRelation(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_relation;
        type_holder_t temp_term;
        bool is_equal = false;
        token_type_e op;

        ret = this->parseTerm(itr, &temp_term);
        if (!ret){
                return false;
        }

        if (((*itr)->type == T_OP_REL_GREATER) ||
                ((*itr)->type == T_OP_REL_LESS) ||
                ((*itr)->type == T_OP_REL_GREATER_EQUAL) ||
                ((*itr)->type == T_OP_REL_LESS_EQUAL) ||
                ((*itr)->type == T_OP_REL_EQUAL) ||
                ((*itr)->type == T_OP_REL_NOT_EQUAL))
        {
                if (((*itr)->type == T_OP_REL_EQUAL) ||
                        ((*itr)->type == T_OP_REL_NOT_EQUAL)){
                        is_equal = true;
                }
                op = (*itr)->type;

                // Then it's a relation?
                this->next_token(itr); // Move to next token
                ret = this->parseRelation(itr, &temp_relation);

                if (temp_term.is_array){
                        error_printf( *itr, "Relation operators are not allowed for arrays \n");
                        return false;
                } else if (type_holder_cmp(temp_relation, temp_term)){
                        // Both are the same
                        if ((temp_relation.type == T_RW_INTEGER) ||
                                (temp_relation.type == T_RW_BOOL) ||
                                (temp_relation.type == T_RW_FLOAT)) {
                                // Good
                                parameter_type->reg_ct = 
                                        this->genRelation( op, temp_term.type, temp_term.reg_ct, temp_relation.type, temp_relation.reg_ct);
                                parameter_type->type = T_RW_BOOL;
                        } else if (temp_relation.type == T_RW_STRING) {
                                if (is_equal){
                                        parameter_type->type = T_RW_BOOL;
                                        parameter_type->reg_ct = 
                                                genRelationStrings(op, temp_term.reg_ct, temp_relation.reg_ct);
                                } else {
                                        error_printf( *itr, "Strings only support == and != relation operators \n");
                                        return false;
                                }
                        } else {
                                error_printf( *itr, "Relation operators are only defined for INT, BOOL and FLOATS \n");
                                return false;
                        }                                
                } else if ( ((temp_relation.type == T_RW_INTEGER) || (temp_relation.type == T_RW_BOOL)) &&
                        ((temp_term.type == T_RW_INTEGER) || (temp_term.type == T_RW_BOOL))) {
                        // Combinations of int and bool are allowed. Default to bool
                        parameter_type->reg_ct = 
                                        this->genRelation( op, temp_term.type, temp_term.reg_ct, temp_relation.type, temp_relation.reg_ct);
                        parameter_type->type = T_RW_BOOL;
                } else {
                        error_printf( *itr, "Types do not match \n"); // TODO print types
                        return false;
                }

        }else{
                *parameter_type = temp_term;
        }


        return ret;
}

/* NOTES FOR TYPE CHECKING
        "The type signatures of a procedures
        arguments must match exactly their
        parameter declaration."
*/
bool Parser::parseProcedureCall(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        symbol_t temp_symbol;
        std::string name;
        std::list<unsigned int> regs;

        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Check that identifier is defined as a procedure
                // set parameter type to return type of procedure
                if (FindSymbol_Helper(itr, &temp_symbol)){
                        if( temp_symbol.type == ST_PROCEDURE ){
                                *parameter_type = temp_symbol.variable_type;
                        }else{
                                error_printf( *itr, "Identifier %s is not callable \n", get_c_string(itr));
                                return false;
                        }
                } else {
                        return false; // Procedure not defined
                }

                name = get_string(itr);
                this->next_token(itr); // Move to next token
        }else {
                // Should never reach here
                ret = false;
                return ret;
        }

        

        if ((*itr)->type == T_SYM_LPAREN){
                this->next_token(itr); // Move to next token
                
                if ((*itr)->type == T_SYM_RPAREN){
                        // Procedure call has no arguments
                        if (temp_symbol.parameter_ct == 0){
                                this->next_token(itr); // Move to next token
                                ret = true;
                        } else {
                                error_printf( *itr, "Expected %d arguments, found none \n", temp_symbol.parameter_ct);
                                return false;
                        }                        
                }else{
                        ret = this->parseArgumentList(itr, temp_symbol, &regs);
                        if ( ret == false){
                                return ret;
                        }

                        if ((*itr)->type == T_SYM_RPAREN){
                                this->next_token(itr); // Move to next token
                                // ret = true; // ret should already be set to true by parseArg
                        }else{
                                error_printf( *itr, "Expected closing parentheses after argument list \n");
                                ret = false;
                        }
                }
        } else {
                // Should never reach here
        }

        parameter_type->reg_ct = this->genProcedureCall(temp_symbol, name, regs);

        return ret;
}

bool Parser::parseArgumentList(std::list<token_t>::iterator *itr, symbol_t procedure_symbol, std::list<unsigned int> *regs)
{
        debug_print_call();
        bool ret = false;
        unsigned int arg_ct = 0;
        bool first = true;

        type_holder_t expr_type;

        do{
                if (first){
                        first = false;
                }else{
                        this->next_token(itr); // Move to next token
                }

                if (arg_ct+1 > procedure_symbol.parameter_ct) {
                        error_printf( *itr, "Procedure call as too many arguments \n");
                        return false;
                }

                ret = this->parseExpression(itr, &expr_type);
                regs->push_back(expr_type.reg_ct);

                if (ret == false){
                        return ret;
                }
                if (expr_type.type != procedure_symbol.parameter_type_arr[arg_ct].type){
                        error_printf( *itr, "Argument %d's type does not match that of procedure call \n", arg_ct+1);
                        return false;
                }

                arg_ct++;
        } while ((*itr)->type == T_SYM_COMMA);

        if (arg_ct < procedure_symbol.parameter_ct) {
                error_printf( *itr, "Procedure call as too few arguments \n");
                return false;
        }
        return ret;
}

bool Parser::parseTerm(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_factor;
        type_holder_t temp_term;
        token_type_e op;

        ret = this->parseFactor(itr, &temp_factor);
        if (ret){
                if (((*itr)->type == T_OP_TERM_DIVIDE) ||
                    ((*itr)->type == T_OP_TERM_MULTIPLY))
                {
                        op = (*itr)->type;
                        // Then it's a term?
                        this->next_token(itr); // Move to next token
                        ret = this->parseTerm(itr, &temp_term);

                        if (type_holder_cmp(temp_factor, temp_term)){
                                if (temp_factor.type == T_RW_INTEGER) {
                                        // Combinations of int and float are allowed
                                        parameter_type->type = T_RW_INTEGER;
                                } else if (temp_factor.type == T_RW_FLOAT){
                                        parameter_type->type = T_RW_FLOAT;
                                } else {
                                        error_printf( *itr, "Terms must both be either integers or floats \n");
                                }
                                parameter_type->reg_ct = genTerm( op, temp_factor.type, temp_factor.reg_ct, temp_term.reg_ct);
                        } else {
                                error_printf( *itr, "Types do not match. Terms must both be either integers or floats \n");
                                return false;
                        }
                }else{
                        *parameter_type = temp_factor;
                }
        }

        return ret;
}

bool Parser::parseName(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        bool global;
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Lookup in symbol table.
                // Set parameter_type
                ret = FindVariableType_Helper(itr, parameter_type, &global); // returns false if not found
                if (!ret){
                        return false;
                }

                this->next_token(itr); // Move to next token
        }else {
                ret = false; // Should never reach here
                return ret;
        }

        if ((*itr)->type == T_SYM_LBRACKET){
                // parse expression
                this->next_token(itr); // Move to next token
                type_holder_t expr_type;
                ret = this->parseExpression(itr, &expr_type);
                if ( ret == false){
                        return ret;
                } else if ( expr_type.type != T_RW_INTEGER){
                        error_printf( *itr, "Array length must be of type integer \n");
                }

                if ((*itr)->type == T_SYM_RBRACKET){
                        this->next_token(itr); // Move to next token
                        // ret = true; // ret should already be set to true by parseExpression
                }else{
                        ret = false;
                        error_printf( *itr, "Expected closing bracket \n");
                }
                // CODEGEN GEP and load then update parameter_type->reg_ct
                unsigned int index_reg = this->genIntToLong(expr_type.reg_ct);
                unsigned int temp_reg = this->genGEP(*parameter_type, index_reg, global);
                parameter_type->reg_ct = this->genLoadReg(parameter_type->type, temp_reg, false);

        } else {
                // load variable
                if (parameter_type->is_array){
                        // TODO What do we do here? I think nothing
                        parameter_type->reg_ct = this->genGEP_Head(*parameter_type, global);
                        // Or
                        // %7 = getelementptr inbounds [2 x i32], [2 x i32]* %2, i64 0, i64 0
                        // %8 = bitcast i32* %7 to i8*
                } if (parameter_type->is_parameter) {
                        // Do nothing
                } else {
                        parameter_type->reg_ct = this->genLoadReg(parameter_type->type, parameter_type->reg_ct, global);
                }
        }
        return ret;
}


/* NOTES FOR TYPE CHECKING
   Inorder for parseExpression to return a type we have to propagate
   the factor type back up through parseRelation and parseArithOp
*/
bool Parser::parseFactor(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        std::list<token_t>::iterator temp_itr = *itr; // Only used in case T_IDENTIFIER

        // Check for L_Paren
        switch ((*itr)->type){
        case T_SYM_LPAREN:
                this->next_token(itr); // Move to next token
                ret = this->parseExpression(itr, parameter_type);
                if ((*itr)->type == T_SYM_RPAREN){
                        this->next_token(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing parentheses \n");
                        ret = false;
                }
                break;
        case T_RW_TRUE:
                genConstant(*itr, parameter_type);
                parameter_type->type = T_RW_BOOL;
                this->next_token(itr); // Move to next token
                ret = true;
                break;
        case T_RW_FALSE:
                genConstant(*itr, parameter_type);
                parameter_type->type = T_RW_BOOL;
                this->next_token(itr); // Move to next token
                ret = true;
                break;
        case T_CONST_STRING:
                genConstant(*itr, parameter_type);
                parameter_type->type = T_RW_STRING;
                this->next_token(itr); // Move to next token
                ret = true;
                break;
        case T_OP_ARITH_MINUS:
                this->next_token(itr); // Move to next token
                if ((*itr)->type == T_CONST_INTEGER){
                        genConstant(*itr, parameter_type, true);
                        parameter_type->type = T_RW_INTEGER;
                        this->next_token(itr); // Move to next token
                        ret = true;
                } else if((*itr)->type == T_CONST_FLOAT){
                        genConstant(*itr, parameter_type, true);
                        parameter_type->type = T_RW_FLOAT;
                        this->next_token(itr); // Move to next token
                        ret = true;
                } else{
                        // TODO modify code generation for this case
                        // multiply by -1 ?
                        ret = this->parseName(itr, parameter_type);
                }
                break;
        case T_CONST_INTEGER:
                genConstant(*itr, parameter_type);
                parameter_type->type = T_RW_INTEGER;
                this->next_token(itr); // Move to next token
                ret = true;
                break;
        case T_CONST_FLOAT:
                genConstant(*itr, parameter_type);
                parameter_type->type = T_RW_FLOAT;
                this->next_token(itr); // Move to next token
                ret = true;
                break;
        case T_IDENTIFIER:
                // Peek ahead to check if it is a procedure call
                this->next_token(itr); // Move to next token
                if ((*itr)->type != T_SYM_LPAREN){
                        // Move pointer back to original position
                        *itr = temp_itr;
                        // Not a procedure call
                        ret = this->parseName(itr, parameter_type);
                }else{
                        // Move pointer back to original position
                        *itr = temp_itr;
                        ret = this->parseProcedureCall(itr, parameter_type);
                }
                break;
        default:
                error_printf( *itr, "Invalid Factor \n");
                ret = false;
        }
        return ret;
}

void Parser::parse(std::list<token_t> token_list)
{
        std::list<token_t>::iterator itr;
        this->itr_end = token_list.end();
        itr = token_list.begin();
        bool ret = this->parseProgram(&itr);
        if (ret){
                printf("Pass\n");
        }else{
                printf("Fail\n");
        }
}