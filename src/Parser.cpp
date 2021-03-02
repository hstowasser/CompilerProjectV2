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

Parser::Parser(Scope* scope)
{
        this->scope = scope;
}

Parser::~Parser()
{
}

void Parser::inc_ptr(std::list<token_t>::iterator *itr)
{
        if (*itr != this->itr_end) {
                (*itr)++;
        }else{
                error_printf( *itr, "Unknown error,  \n");
        }
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
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
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected Semicolon \n");
                        return false;
                }
        }

        // check for "begin"
        if ((*itr)->type == T_RW_BEGIN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"begin\" in program body \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected Semicolon \n");
                        return false;
                }
        }

        // check for "end" "program"
        if ((*itr)->type == T_RW_END){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                if ((*itr)->type == T_RW_PROGRAM){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
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

bool Parser::parseDestination(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        // check identifier
        if ((*itr)->type == T_IDENTIFIER){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else{
                return false; // Not a valid destination
        }


        // if open bracket
        if ((*itr)->type == T_SYM_LBRACKET){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                // then parse expression
                ret = this->parseExpression(itr);
                if ( !ret ){
                        return false;
                }

                // check close bracket
                if ((*itr)->type == T_SYM_RBRACKET){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing bracket \n");
                        return false;
                }
        }
        

        return ret;
}

bool Parser::parseAssignmentStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        ret = this->parseDestination(itr);
        if (!ret){
                return false; // Not an assignment statement
        }

        // check for assignment op :=
        if ((*itr)->type == T_OP_ASIGN_EQUALS){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \":=\" \n");
                return false;
        }

        // parse expression
        ret = this->parseExpression(itr); // TODO check that destination type matches expression type

        return ret; 
}

bool Parser::parseIfStatement(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        if ((*itr)->type == T_RW_IF){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; //Not an if statement
        }

        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"(\" after IF \n");
                return false;
        }

        ret = this->parseExpression(itr); // TODO Check that expression type is either int or bool
        if (!ret){
                return false;
        }

        if ((*itr)->type == T_SYM_RPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \")\" before THEN \n");
                return false;
        }

        if ((*itr)->type == T_RW_THEN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected THEN \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }

        // if "else"
        if ((*itr)->type == T_RW_ELSE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                // parse statements
                while ( this->parseStatement(itr) ){
                        // check for ;
                        if ((*itr)->type == T_SYM_SEMICOLON){
                                debug_print_token(**itr);
                                this->inc_ptr(itr); // Move to next token
                        }else{
                                error_printf( *itr, "Expected semicolon \n");
                                return false;
                        }
                }
        }

        if ((*itr)->type == T_RW_END){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                if ((*itr)->type == T_RW_IF){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
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

        if ((*itr)->type == T_RW_FOR){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; //Not a for statement
        }

        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"(\" after FOR \n");
                return false;
        }

        ret = this->parseAssignmentStatement(itr);
        if (!ret){
                return false;
        }

        if ((*itr)->type == T_SYM_SEMICOLON){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected semicolon \n");
                return false;
        }

        ret = this->parseExpression(itr); // TODO Check that expression is bool or int
        if (!ret){
                return false;
        }

        if ((*itr)->type == T_SYM_RPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \")\" \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }


        if ((*itr)->type == T_RW_END){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                if ((*itr)->type == T_RW_FOR){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
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

        if ((*itr)->type == T_RW_RETURN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; // Not a return statement
        }

        ret = this->parseExpression(itr);

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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                global = true;
        }

        if ((ret = this->parseProcedureDeclaration(itr, global))){
                // Procedure Declaration
        }else if ((ret = this->parseVariableDeclaration(itr, global))){
                // Variable Declaration
        }else if ((ret = this->parseTypeDeclaration(itr, global))){
                // Type Declaration
        } // else no valid declaration

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
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }

        // check for "begin"
        if ((*itr)->type == T_RW_BEGIN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"BEGIN\" in procedure body \n");
                return false;
        }

        // parse statements
        while ( this->parseStatement(itr) ){
                // check for ;
                if ((*itr)->type == T_SYM_SEMICOLON){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected semicolon \n");
                        return false;
                }
        }

        // check for "end" "procedure"
        if ((*itr)->type == T_RW_END){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                if ((*itr)->type == T_RW_PROCEDURE){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; // Not a procedure header
        }

        // check identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol.type = ST_PROCEDURE;
                name = *(*itr)->getStringValue();

                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"PROCEDURE\" \n");
                return false;
        }

        // check colon
        if ((*itr)->type == T_SYM_COLON){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                ret = this->parseParameterList(itr, global, &symbol);

                if ((*itr)->type == T_SYM_RPAREN){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                } else {
                        error_printf( *itr, "Expected closing parentheses after procedure parameters \n");
                        return false;
                }
        }else{
                error_printf( *itr, "Expected parentheses after procedure identifier \n");
                return false;
        }

        if (global){
                this->scope->AddGlobalSymbol(name, symbol);
                this->scope->PushScope(name);
        }else{
                // To allow for recursive calls add procedure symbol to both current and next scope
                this->scope->AddSymbol(name, symbol);
                this->scope->PushScope(name);
                this->scope->AddSymbol(name, symbol);
        }

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
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }
                ret = this->parseParameter(itr, global, &temp_parameter_type); // TODO Test type tracking
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; // Not a variable declaration
        }

        // check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol.type = ST_VARIABLE;
                name = *(*itr)->getStringValue();

                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"VARIABLE\" \n");
                return false;
        }

        // check for colon
        if ((*itr)->type == T_SYM_COLON){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                // parse bound
                if ((*itr)->type == T_CONST_INTEGER){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected bound inside of [] \n");
                        return false;
                }

                // check for close bracket
                if ((*itr)->type == T_SYM_RBRACKET){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing bracket \n");
                        return false;
                }
        }

        if (global){
                this->scope->AddGlobalSymbol(name, symbol);
        }else{
                this->scope->AddSymbol(name, symbol);
        }

        return ret;
}

bool Parser::parseTypeDeclaration(std::list<token_t>::iterator *itr, bool global)
{
        debug_print_call();
        bool ret = false;
        std::string name;
        symbol_t symbol;

        // check "type" tag
        if ((*itr)->type == T_RW_TYPE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; // not an error, just not a type
        }

        // check identifier
        if ((*itr)->type == T_IDENTIFIER){                
                symbol.type = ST_TYPE;
                name = *(*itr)->getStringValue();
                
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after  \"TYPE\" \n");
                return false;
        }

        // check "is" tag
        if ((*itr)->type == T_RW_IS){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected \"IS\" after type identifier \n");
                return false;
        }

        // parse typemark
        ret = this->parseTypeDef(itr, global, &symbol);
        if( !ret){
                return false;
        }

        if (global){
                this->scope->AddGlobalSymbol(name, symbol);
        }else{
                this->scope->AddSymbol(name, symbol);
        }

        return ret;
}

bool Parser::parseTypeDef(std::list<token_t>::iterator *itr, bool global /*= false*/, symbol_t* symbol /*= NULL*/)
{
        debug_print_call();
        bool ret = false;

        if (((*itr)->type == T_RW_ENUM)){ // if enum
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                // check for LPAREN
                if ((*itr)->type == T_SYM_LBRACE){
                        // Special case, don't move to next token till do/while
                }else{
                        error_printf( *itr, "Expected opening brace after \"enum\" \n");
                        return false;
                }

                // loop through enum identifiers
                unsigned int e_index = 0;
                do{
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token

                        // Check identifier
                        if ((*itr)->type == T_IDENTIFIER){
                                // Add to symbol table with associated index
                                symbol_t temp_symbol;
                                temp_symbol.type = ST_ENUM_CONST;
                                temp_symbol.enum_index = e_index; // Integer value of the enum
                                if (global){
                                        this->scope->AddGlobalSymbol(*(*itr)->getStringValue(), temp_symbol);
                                }else{
                                        this->scope->AddSymbol(*(*itr)->getStringValue(), temp_symbol);
                                }
                                debug_print_token(**itr);
                                this->inc_ptr(itr); // Move to next token                       
                        }else{
                                error_printf( *itr, "Expected identifier in \"ENUM\" declaration \n");
                                return false;
                        }
                        e_index++;

                }while ((*itr)->type == T_SYM_COMMA);

                // check for RPAREN
                if ((*itr)->type == T_SYM_RBRACE){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        ret = true;
                }else{
                        error_printf( *itr, "Expected closing brace after \"ENUM\" declaration \n");
                        return false;
                }
        }else{
                // parseTypeMark
                ret = parseTypeMark(itr, global, symbol);
                return false;
        }

        return ret;
}

/**     This gets  called by parseProcedureHeader, parseVariableDeclaration and parseTypeMark
 *      if it is called by parseTypeMark4
 * 
 */
bool Parser::parseTypeMark(std::list<token_t>::iterator *itr, bool global /*= false*/, symbol_t* symbol /*= NULL*/)
{
        debug_print_call();
        bool ret = false;

        if (((*itr)->type == T_RW_INTEGER) ||
            ((*itr)->type == T_RW_FLOAT) ||
            ((*itr)->type == T_RW_STRING) ||
            ((*itr)->type == T_RW_BOOL) ||
            ((*itr)->type == T_RW_ENUM) ||
            ((*itr)->type == T_IDENTIFIER))
        {
                if (symbol != NULL){
                        symbol->variable_type.type = (*itr)->type;
                }
        }

        // check for integer or float or string or bool
        // then get identifier
        if (((*itr)->type == T_RW_INTEGER) ||
            ((*itr)->type == T_RW_FLOAT) ||
            ((*itr)->type == T_RW_STRING) ||
            ((*itr)->type == T_RW_BOOL))
        {
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_IDENTIFIER){
                // Check symbol table for type. If exists set symbol->variable_type.ptr
                bool success;
                std::map<std::string,symbol_t>::iterator temp;
                temp = this->scope->Find(*((*itr)->getStringValue()), &success);
                if (success){
                        symbol->variable_type.itr = temp;
                } else {
                        error_printf( *itr, "Type %s is not defined \n",(*itr)->getStringValue()->c_str());
                        return false;
                }

                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Code must start with \"PROGRAM\" definition\n");
                return false;
        }

        // parse identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Create scope for program
                this->scope->PushScope(*(*itr)->getStringValue());

                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                error_printf( *itr, "Expected identifier after \"PROGRAM\" \n");
                return false;
        }

        // check for "is"
        if ((*itr)->type == T_RW_IS){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
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
bool Parser::parseExpression(std::list<token_t>::iterator *itr, type_holder_t* parameter_type /*= Null*/)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_arithop;
        type_holder_t temp_expression;

        // check for not
        if ((*itr)->type == T_OP_LOGI_NOT){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }
        
        ret = this->parseArithOp(itr, &temp_arithop); // TODO add type checking
        if (ret){
                if (((*itr)->type == T_OP_LOGI_AND) ||
                    ((*itr)->type == T_OP_LOGI_OR))
                {
                        debug_print_token(**itr);
                        // Then it's an Expression?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseExpression(itr, &temp_expression);

                        if (parameter_type != NULL) {
                                if (type_holder_cmp(temp_expression, temp_arithop)){
                                        // Both are the same
                                        *parameter_type = temp_arithop;
                                } else if ( ((temp_expression.type == T_RW_INTEGER) || (temp_expression.type == T_RW_BOOL)) &&
                                        ((temp_expression.type == T_RW_INTEGER) || (temp_expression.type == T_RW_BOOL))) {
                                        // Combinations of int and bool are allowed
                                        parameter_type->type = T_RW_BOOL; // Default to bool
                                } else {
                                        error_printf( *itr, "Types do not match \n"); // TODO print types
                                        return false;
                                }
                        }
                        
                }else{
                        if (parameter_type != NULL) {
                                *parameter_type = temp_arithop;
                        }                        
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

        ret = this->parseRelation(itr, &temp_relation); // TODO test type checking
        if (ret){
                if (((*itr)->type == T_OP_ARITH_MINUS) ||
                    ((*itr)->type == T_OP_ARITH_PLUS))
                {
                        debug_print_token(**itr);
                        // Then it's an ArithOp?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseArithOp(itr, &temp_arithop);

                        if (type_holder_cmp(temp_relation, temp_arithop)){
                                // Both are the same
                                *parameter_type = temp_relation;
                        } else if ( ((temp_relation.type == T_RW_INTEGER) || (temp_relation.type == T_RW_BOOL)) &&
                                ((temp_arithop.type == T_RW_INTEGER) || (temp_arithop.type == T_RW_BOOL))) {
                                // Combinations of int and bool are allowed
                                parameter_type->type = T_RW_INTEGER; // Default to int
                        } else if ( ((temp_relation.type == T_RW_INTEGER) || (temp_relation.type == T_RW_FLOAT)) &&
                                ((temp_arithop.type == T_RW_INTEGER) || (temp_arithop.type == T_RW_FLOAT))) {
                                // Combinations of int and float are allowed
                                parameter_type->type = T_RW_FLOAT; // Default to float
                        } else {
                                error_printf( *itr, "Types do not match \n"); // TODO print types
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

        ret = this->parseTerm(itr, &temp_term);
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
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseRelation(itr, &temp_relation);

                        if (type_holder_cmp(temp_relation, temp_term)){
                                // Both are the same
                                *parameter_type = temp_term;
                        } else if ( ((temp_relation.type == T_RW_INTEGER) || (temp_relation.type == T_RW_BOOL)) &&
                                ((temp_term.type == T_RW_INTEGER) || (temp_term.type == T_RW_BOOL))) {
                                // Combinations of int and bool are allowed
                                parameter_type->type = T_RW_BOOL; // Default to bool
                        } else {
                                error_printf( *itr, "Types do not match \n"); // TODO print types
                                return false;
                        }
                }else{
                        *parameter_type = temp_term;
                }
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

        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Check that identifier is defined as a procedure
                // set parameter type to return type of procedure
                bool success;
                std::map<std::string,symbol_t>::iterator temp;
                temp = this->scope->Find(*((*itr)->getStringValue()), &success);
                if (success){
                        if (temp->second.type == ST_PROCEDURE){
                                *parameter_type = temp->second.variable_type;
                        }else{
                                error_printf( *itr, "Identifier %s is not callable \n",(*itr)->getStringValue()->c_str());
                                return false;
                        }                        
                } else {
                        error_printf( *itr, "Procedure %s is not defined \n",(*itr)->getStringValue()->c_str());
                        return false;
                }

                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else {
                // Should never reach here
                ret = false;
                return ret;
        }


        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                
                if ((*itr)->type == T_SYM_RPAREN){
                        // Procedure call has no arguments
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        ret = true;
                }else{
                        ret = this->parseArgumentList(itr);
                        if ( ret == false){
                                return ret;
                        }

                        if ((*itr)->type == T_SYM_RPAREN){
                                debug_print_token(**itr);
                                this->inc_ptr(itr); // Move to next token
                                // ret = true; // ret should already be set to true by parseArg
                        }else{
                                error_printf( *itr, "Expected closing parentheses after argument list \n");
                                ret = false;
                        }
                }
        } else {
                // Should never reach here
        }
        return ret;
}

bool Parser::parseArgumentList(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        // Parse expression
        ret = this->parseExpression(itr);
        if (ret == false){
                return ret;
        }

        // if comma, keep parsing Argument List
        if ((*itr)->type == T_SYM_COMMA){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                //Parse ArgumentList
                ret = this->parseArgumentList(itr);
        }
        return ret;
}

bool Parser::parseTerm(std::list<token_t>::iterator *itr, type_holder_t* parameter_type)
{
        debug_print_call();
        bool ret = false;
        type_holder_t temp_factor;
        type_holder_t temp_term;

        ret = this->parseFactor(itr, &temp_factor);
        if (ret){
                if (((*itr)->type == T_OP_TERM_DIVIDE) ||
                    ((*itr)->type == T_OP_TERM_MULTIPLY))
                {
                        debug_print_token(**itr);
                        // Then it's a term?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseTerm(itr, &temp_term);

                        if (type_holder_cmp(temp_factor, temp_term)){
                                // Both are the same
                                *parameter_type = temp_factor;
                        } else if ( ((temp_factor.type == T_RW_INTEGER) || (temp_factor.type == T_RW_FLOAT)) &&
                                ((temp_term.type == T_RW_INTEGER) || (temp_term.type == T_RW_FLOAT))) {
                                // Combinations of int and float are allowed
                                parameter_type->type = T_RW_FLOAT; // Default to float
                        } else {
                                error_printf( *itr, "Types do not match \n"); // TODO print types
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
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                // Lookup in symbol table.
                // Set parameter_type
                bool success;
                std::map<std::string,symbol_t>::iterator temp;
                temp = this->scope->Find(*((*itr)->getStringValue()), &success);
                if (success){
                        *parameter_type = temp->second.variable_type;
                } else {
                        error_printf( *itr, "Name %s is not defined \n",(*itr)->getStringValue()->c_str());
                        return false;
                }

                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else {
                ret = false; // Should never reach here
                return ret;
        }

        if ((*itr)->type == T_SYM_LBRACKET){
                // parse expression
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                type_holder_t expr_type;
                ret = this->parseExpression(itr, &expr_type);
                if ( ret == false){
                        return ret;
                } else if ( expr_type.type != T_RW_INTEGER){
                        error_printf( *itr, "Array length must be of type integer \n");
                }

                if ((*itr)->type == T_SYM_RBRACKET){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        // ret = true; // ret should already be set to true by parseExpression
                }else{
                        ret = false;
                        error_printf( *itr, "Expected closing bracket \n");
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
        // Check for L_Paren
        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = this->parseExpression(itr, parameter_type);
                if ((*itr)->type == T_SYM_RPAREN){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing parentheses \n");
                        ret = false;
                }
        }else if ((*itr)->type == T_RW_TRUE){
                parameter_type->type = T_RW_BOOL;
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_FALSE){
                parameter_type->type = T_RW_BOOL;
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_CONST_STRING){
                parameter_type->type = T_RW_STRING;
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_OP_ARITH_MINUS){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                if ((*itr)->type == T_CONST_INTEGER ||
                    (*itr)->type == T_CONST_FLOAT){
                        if ((*itr)->type == T_CONST_INTEGER){
                                parameter_type->type = T_RW_INTEGER;
                        }else{
                                parameter_type->type = T_RW_FLOAT;
                        }
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        ret = true;
                } else if (!ret){
                        ret = this->parseName(itr, parameter_type);
                }
        }else if ((*itr)->type == T_CONST_INTEGER ||
                 (*itr)->type == T_CONST_FLOAT){

                if ((*itr)->type == T_CONST_INTEGER){
                        parameter_type->type = T_RW_INTEGER;
                }else{
                        parameter_type->type = T_RW_FLOAT;
                }
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_IDENTIFIER){
                std::list<token_t>::iterator temp_itr = *itr;

                // Peek ahead to check if it is a procedure call
                this->inc_ptr(itr); // Move to next token
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
        }else{
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