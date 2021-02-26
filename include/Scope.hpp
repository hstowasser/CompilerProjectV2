#pragma once
#include "Symbol.hpp"
#include <string>
#include <map>

class Symbol; // Work-around for circular dependencies

class Scope{
private:
        std::map<std::string,Symbol> symbol_table;
public:
};
