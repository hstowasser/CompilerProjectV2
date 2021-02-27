#include "Parser.hpp"
#include "Token.hpp"

#if 0
#include <stdio.h>
#define debug_print_call() printf("%s\n", __FUNCTION__)
#else
#define debug_print_call()
#endif

#if 0
#define debug_print_token(itr) print_token(itr)
#else
#define debug_print_token(itr)
#endif

#include <stdarg.h>

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
        ret = this->parseExpression(itr);

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

        ret = this->parseExpression(itr);
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

        ret = this->parseExpression(itr);
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

        return ret;
}

bool Parser::parseProcedureHeader(std::list<token_t>::iterator *itr, bool global)
{
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
                
                symbol_t symbol;
                symbol.type = ST_PROCEDURE;
                if (global){
                        this->scope->AddGlobalSymbol(*(*itr)->getStringValue(), symbol);
                        this->scope->PushScope(*(*itr)->getStringValue());
                }else{
                        // To allow for recursive calls add procedure symbol to both current and next scope
                        this->scope->AddSymbol(*(*itr)->getStringValue(), symbol);
                        this->scope->PushScope(*(*itr)->getStringValue());
                        this->scope->AddSymbol(*(*itr)->getStringValue(), symbol);
                }

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
        ret = this->parseTypeMark(itr);
        if (!ret){
                return false;
        }

        // check lparen, parseParameterList, check rparen
        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token

                ret = this->parseParameterList(itr);

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

        return ret;
}

bool Parser::parseParameterList(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        bool first = true;
        do{
                if (!first){
                        // Skip comma
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }
                ret = this->parseParameter(itr);
                first = false;
        } while(ret && (*itr)->type == T_SYM_COMMA);

        return ret;
}

bool Parser::parseVariableDeclaration(std::list<token_t>::iterator *itr, bool global)
{
        debug_print_call();
        bool ret = false;

        // check for "variable"
        if ((*itr)->type == T_RW_VARIABLE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; // Not a variable declaration
        }

        // check for identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol_t symbol;
                symbol.type = ST_VARIABLE;
                if (global){
                        this->scope->AddGlobalSymbol(*(*itr)->getStringValue(), symbol);
                }else{
                        this->scope->AddSymbol(*(*itr)->getStringValue(), symbol);
                }
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
        ret = this->parseTypeMark(itr);
        if (!ret){
                return false;
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

        return ret;
}

bool Parser::parseTypeDeclaration(std::list<token_t>::iterator *itr, bool global)
{
        debug_print_call();
        bool ret = false;

        // check "type" tag
        if ((*itr)->type == T_RW_TYPE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }else{
                return false; // not an error, just not a type
        }

        // check identifier
        if ((*itr)->type == T_IDENTIFIER){
                symbol_t symbol;
                symbol.type = ST_TYPE;
                if (global){
                        this->scope->AddGlobalSymbol(*(*itr)->getStringValue(), symbol);
                }else{
                        this->scope->AddSymbol(*(*itr)->getStringValue(), symbol);
                }
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
        ret = this->parseTypeMark(itr);

        return ret;
}

/**     This gets  called by parseProcedureHeader, parseVariableDeclaration and parseTypeMark
 *      if it is called by parseTypeMark4
 * 
 */
bool Parser::parseTypeMark(std::list<token_t>::iterator *itr, bool global /*= false*/)
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
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_IDENTIFIER){
                // TODO check symbol table for type
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if (((*itr)->type == T_RW_ENUM)){ // if enum
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
                                // Unless Wilsey says otherwise
                                symbol_t symbol;
                                symbol.type = ST_ENUM_CONST;
                                symbol.enum_index = e_index; // Integer value of the enum
                                if (global){
                                        this->scope->AddGlobalSymbol(*(*itr)->getStringValue(), symbol);
                                }else{
                                        this->scope->AddSymbol(*(*itr)->getStringValue(), symbol);
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
        }
        // else return false

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

bool Parser::parseExpression(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        // check for not
        if ((*itr)->type == T_OP_LOGI_NOT){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
        }
        
        ret = this->parseArithOp(itr);
        if (ret){
                if (((*itr)->type == T_OP_LOGI_AND) ||
                    ((*itr)->type == T_OP_LOGI_OR))
                {
                        debug_print_token(**itr);
                        // Then it's an Expression?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseExpression(itr);
                }
        }
        return ret;
}

bool Parser::parseArithOp(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        ret = this->parseRelation(itr);
        if (ret){
                if (((*itr)->type == T_OP_ARITH_MINUS) ||
                    ((*itr)->type == T_OP_ARITH_PLUS))
                {
                        debug_print_token(**itr);
                        // Then it's an ArithOp?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseArithOp(itr);
                }
        }
        return ret;
}

bool Parser::parseRelation(std::list<token_t>::iterator *itr)
{
        debug_print_call();
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
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseRelation(itr);
                }
        }
        return ret;
}

bool Parser::parseProcedureCall(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;

        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
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

bool Parser::parseTerm(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        ret = this->parseFactor(itr);
        if (ret){
                if (((*itr)->type == T_OP_TERM_DIVIDE) ||
                    ((*itr)->type == T_OP_TERM_MULTIPLY))
                {
                        debug_print_token(**itr);
                        // Then it's a term?
                        this->inc_ptr(itr); // Move to next token
                        ret = this->parseTerm(itr);
                }
        }
        return ret;
}

bool Parser::parseName(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        // Check for identifier
        if ((*itr)->type == T_IDENTIFIER){
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
                ret = this->parseExpression(itr);
                if ( ret == false){
                        return ret;
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

bool Parser::parseFactor(std::list<token_t>::iterator *itr)
{
        debug_print_call();
        bool ret = false;
        // Check for L_Paren
        if ((*itr)->type == T_SYM_LPAREN){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = this->parseExpression(itr);
                if ((*itr)->type == T_SYM_RPAREN){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                }else{
                        error_printf( *itr, "Expected closing parentheses \n");
                        ret = false;
                }
        }else if ((*itr)->type == T_RW_TRUE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_RW_FALSE){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_CONST_STRING){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                ret = true;
        }else if ((*itr)->type == T_OP_ARITH_MINUS){
                debug_print_token(**itr);
                this->inc_ptr(itr); // Move to next token
                if ((*itr)->type == T_CONST_INTEGER ||
                    (*itr)->type == T_CONST_FLOAT){
                        debug_print_token(**itr);
                        this->inc_ptr(itr); // Move to next token
                        ret = true;
                } else if (!ret){
                        ret = this->parseName(itr);
                }
        }else if ((*itr)->type == T_CONST_INTEGER ||
                 (*itr)->type == T_CONST_FLOAT){
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
                        ret = this->parseName(itr);
                }else{
                        // Move pointer back to original position
                        *itr = temp_itr;
                        ret = this->parseProcedureCall(itr);
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