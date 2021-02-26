#pragma once
#include <string>
#include <map>
#include <list>

typedef enum {
        ST_UNKNOWN,
        ST_VARIABLE,
        ST_PROCEDURE,
        ST_TYPE
} SymbolType_e;

typedef struct {
        SymbolType_e type;
        union value
        {
                int variable_type; // Could also double as function return type
                // TODO what to do with function parameters
        };
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

        void PrintScope(); //For Debugging

};
