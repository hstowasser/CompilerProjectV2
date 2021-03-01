#include "Scope.hpp"
#include <iostream>

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

std::map<std::string,symbol_t>::iterator Scope::Find(std::string name, bool* success){
    std::map<std::string,symbol_t>::iterator ret;
    ret = this->symbol_tables[this->current_scope_name].find(name);
    
    if (ret != this->symbol_tables[this->current_scope_name].end()){
        *success = true;
    }else{
        ret = this->global_symbol_table.find(name);
        if (ret == this->global_symbol_table.end())
        {
            *success = false;
        }else{
            *success = true;
        }
    }
    return ret;
}

void Scope::PrintScope()
{
    // Print Global Scope
    std:: cout << "Global Scope:" << std::endl;
    symbol_table_t::iterator it;
    for (it = this->global_symbol_table.begin(); it != this->global_symbol_table.end(); it++)
    {
        std::cout << "\t" << it->first
            << std::endl;
    }

    // Print Local Scopes
    std::map<std::string, symbol_table_t>::iterator itm;
    for (itm = this->symbol_tables.begin(); itm != this->symbol_tables.end(); itm++)
    {
        std::cout << "Local Scope: " << itm->first << std::endl;

        symbol_table_t::iterator it;
        for (it = itm->second.begin(); it != itm->second.end(); it++)
        {
            std::cout << "\t" << it->first
                << std::endl;
        }
    }
}