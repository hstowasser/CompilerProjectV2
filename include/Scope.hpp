#pragma once
#include <string>
#include <map>
#include <list>
#include "Token.hpp"
#include "Symbol.hpp"

typedef enum {
        ST_UNKNOWN,
        ST_VARIABLE,
        ST_PROCEDURE,
        ST_ENUM_CONST,
        ST_TYPE
} SymbolType_e;


typedef struct _type_holder_t type_holder_t;
typedef struct _symbol_t symbol_t;

bool type_holder_cmp(type_holder_t a,  type_holder_t b);

typedef struct _type_holder_t{
        token_type_e type; // T_RW_INTEGER T_RW_FLOAT T_RW_BOOL T_RW_ENUM T_RW_STRING RW_IDENTIFIER are the only valid values

        // IF RW_IDENTIFIER
        std::map<std::string,symbol_t>::iterator itr; // Pointer to custom type symbol
} type_holder_t;

typedef struct _symbol_t{
        SymbolType_e type;
        type_holder_t variable_type; // Could also double as function return type
        unsigned int enum_index; // for ST_ENUM_CONST

        // TODO what to do with function parameters. Will need to allocate memory
        unsigned int parameter_ct = 0;
        type_holder_t * parameter_type_arr = NULL; // Will need to account for memory leak
} symbol_t;

typedef std::map<std::string,symbol_t> symbol_table_t; // rename to symbol table?

class Scope{
private:
        symbol_table_t global_symbol_table;

        std::map<std::string, symbol_table_t> symbol_tables; // Need some kind of associated identifier

        std::list<std::string> scope_stack;

        std::string current_scope_name;
public:
        Scope();
        // TODO add destructor that deletes memory allocated to parameter_type_arr

        // Adds a symbol to the current scope
        void AddSymbol(std::string, symbol_t symbol); // Consider adding redeclaration error output
        void AddGlobalSymbol(std::string, symbol_t symbol);

        void PushScope(std::string);
        void PopScope();

        std::map<std::string,symbol_t>::iterator Find(std::string, bool* success); //returns 

        // void SetValueType(std::string, );

        void PrintScope(); //For Debugging

};
