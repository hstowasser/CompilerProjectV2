#include "Scope.hpp"


Scope::Scope()
{
    this->current_scope_name = "GLOBAL";
}

void Scope::PushScope(std::string name)
{
    std::string new_scope_name = this->current_scope_name + name;
    symbol_table_t temp;
    this->symbol_tables[new_scope_name] = temp;

    this->current_scope_name = new_scope_name;

    this->scope_stack.push_back(new_scope_name);
}

void Scope::PopScope()
{
    this->current_scope_name = this->scope_stack.back();
    this->scope_stack.pop_back();
}

void Scope::AddSymbol(std::string name, symbol_t symbol)
{
    std::string scope_tag = this->current_scope_name;
    this->symbol_tables[scope_tag][name] = symbol;
}

void Scope::AddGlobalSymbol(std::string name, symbol_t symbol)
{
    this->global_symbol_table[name] = symbol;
}