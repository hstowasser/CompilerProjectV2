#pragma once
#include "Scope.hpp"

class Scope; // Work-around for circular dependencies

class Symbol{
public:
        //SymbolEntry();

        enum SymbolType{
                ST_UNKNOWN,
                ST_VARIABLE,
                ST_PROCEDURE
        };
private:
        enum SymbolType _symboltype;
        union _value
        {
                int variable_type;
                Scope *local_scope;
        };
        
};