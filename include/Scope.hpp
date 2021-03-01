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

typedef struct _type_holder_t{
        token_type_e type; // INT FLOAT BOOL ENUM STRING IDENTIFIER are the only valid values
        // IF CUSTOM
        std::map<std::string,symbol_t>::iterator ptr; // Pointer to custom type symbol
} type_holder_t;

typedef struct _symbol_t{
        SymbolType_e type;
        //union {
                type_holder_t variable_type; // Could also double as function return type
                unsigned int enum_index; // for ST_ENUM_CONST
        //};

        // TODO what to do with function parameters. Will need to allocate memory
        unsigned int parameter_ct;
        type_holder_t * parameter_types; // Will need to account for memory leak
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

        // Adds a symbol to the current scope
        void AddSymbol(std::string, symbol_t symbol); // Consider adding redeclaration error output
        void AddGlobalSymbol(std::string, symbol_t symbol);

        void PushScope(std::string);
        void PopScope();

        // void SetValueType(std::string, );

        void PrintScope(); //For Debugging

};
