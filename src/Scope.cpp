#include "Scope.hpp"
#include <iostream>

bool type_holder_cmp(type_holder_t a,  type_holder_t b)
{
    if(a.type == b.type){
        if (a.is_array == b.is_array){
            if (a.is_array){
                if (a.array_length == b.array_length){
                    return true; // Types and lengths match
                }else{
                    return false; // Array lengths do not match
                }
            } else {
                return true; // types match and neither are arrays
            }            
        }else{
            return false; // one is an array and the other is not
        }        
    }else{
        return false; // Types do not match
    }
}

Scope::Scope()
{
    this->current_scope_name = "GLOBAL";

    // INITIALIZE BUILT IN FUNCTIONS

    this->writeCode("@.str0 = private unnamed_addr constant [4 x i8] c\"%d\\0A\\00\", align 1", true);
    this->writeCode("@.str1 = private unnamed_addr constant [4 x i8] c\"%f\\0A\\00\", align 1", true);
    this->writeCode("@.str2 = private unnamed_addr constant [4 x i8] c\"%s\\0A\\00\", align 1", true);

    this->writeCode("declare void @printf( i8*, ...)", true);
    this->writeCode("declare void @scanf( i8*, ...)", true);
    this->writeCode("declare i32 @strcmp(i8*, i8*)", true);
    this->writeCode("declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1", true);

    // putBool(bool Value): bool
    symbol_t putBool;
    std::string putBoolName = "PUTBOOL";
    putBoolName += '\0';
    putBool.type = ST_PROCEDURE;
    putBool.parameter_ct = 1;
    putBool.parameter_type_arr = (type_holder_t*)calloc(putBool.parameter_ct, sizeof(type_holder_t));
    putBool.parameter_type_arr[0].type = T_RW_BOOL;
    putBool.variable_type.type = T_RW_BOOL;
    this->AddGlobalSymbol(putBoolName,putBool);
    this->writeCode("define i8 @PUTBOOL0(i8 %0) {", true);
    this->writeCode("  %2 = getelementptr [4 x i8], [4 x i8]* @.str0, i64 0, i64 0", true);
    this->writeCode("  call void (i8*, ...) @printf( i8* %2, i8 %0)", true);
    this->writeCode("  ret i8 0", true);
    this->writeCode("}", true);

    // putInteger(integer Value): bool
    symbol_t putInteger;
    std::string putIntegerName = "PUTINTEGER";
    putIntegerName += '\0';
    putInteger.type = ST_PROCEDURE;
    putInteger.parameter_ct = 1;
    putInteger.parameter_type_arr = (type_holder_t*)calloc(putInteger.parameter_ct, sizeof(type_holder_t));
    putInteger.parameter_type_arr[0].type = T_RW_INTEGER;
    putInteger.variable_type.type = T_RW_BOOL; //Return type
    this->AddGlobalSymbol(putIntegerName,putInteger);
    this->writeCode("define i8 @PUTINTEGER0(i32 %0) {", true);
    this->writeCode("  %2 = getelementptr [4 x i8], [4 x i8]* @.str0, i64 0, i64 0", true);
    this->writeCode("  call void (i8*, ...) @printf( i8* %2, i32 %0)", true);
    this->writeCode("  ret i8 0", true);
    this->writeCode("}", true);

    // putFloat(float Value): bool
    symbol_t putFloat;
    std::string putFloatName = "PUTFLOAT";
    putFloatName += '\0';
    putFloat.type = ST_PROCEDURE;
    putFloat.parameter_ct = 1;
    putFloat.parameter_type_arr = (type_holder_t*)calloc(putFloat.parameter_ct, sizeof(type_holder_t));
    putFloat.parameter_type_arr[0].type = T_RW_FLOAT;
    putFloat.variable_type.type = T_RW_BOOL; //Return type
    this->AddGlobalSymbol(putFloatName,putFloat);
    this->writeCode("define i8 @PUTFLOAT0(float %0) {", true);
    this->writeCode("  %2 = fpext float %0 to double", true);
    this->writeCode("  %3 = getelementptr [4 x i8], [4 x i8]* @.str1, i64 0, i64 0", true);
    this->writeCode("  call void (i8*, ...) @printf( i8* %3, double %2)", true);
    this->writeCode("  ret i8 0", true);
    this->writeCode("}", true);

    // putString(string Value): bool
    symbol_t putString;
    std::string putStringName = "PUTSTRING";
    putStringName += '\0';
    putString.type = ST_PROCEDURE;
    putString.parameter_ct = 1;
    putString.parameter_type_arr = (type_holder_t*)calloc(putString.parameter_ct, sizeof(type_holder_t));
    putString.parameter_type_arr[0].type = T_RW_STRING;
    putString.variable_type.type = T_RW_BOOL; //Return type
    this->AddGlobalSymbol(putStringName,putString);
    this->writeCode("define i8 @PUTSTRING0(i8* %0) {", true);
    this->writeCode("	%2 = getelementptr [4 x i8], [4 x i8]* @.str2, i64 0, i64 0", true);
    this->writeCode("	call void (i8*, ...) @printf( i8* %2, i8* %0)", true);
    this->writeCode("	ret i8 0", true);
    this->writeCode("}", true);

    // sqrt(integer Value): float
    symbol_t sqrt;
    sqrt.type = ST_PROCEDURE;
    sqrt.parameter_ct = 1;
    sqrt.parameter_type_arr = (type_holder_t*)calloc(sqrt.parameter_ct, sizeof(type_holder_t));
    sqrt.parameter_type_arr[0].type = T_RW_INTEGER;
    sqrt.variable_type.type = T_RW_FLOAT; //Return type
    this->AddGlobalSymbol("SQRT",sqrt);

    // getBool(): bool Value
    symbol_t getBool;
    std::string getBoolName = "GETBOOL";
    getBoolName += '\0';
    getBool.type = ST_PROCEDURE;
    getBool.variable_type.type = T_RW_BOOL;
    this->AddGlobalSymbol(getBoolName,getBool);

    // getInteger(): integer Value
    symbol_t getInteger;
    std::string getIntegerName = "GETINTEGER";
    getIntegerName += '\0';
    getInteger.type = ST_PROCEDURE;
    getInteger.variable_type.type = T_RW_INTEGER;
    this->AddGlobalSymbol(getIntegerName,getInteger);

    // getFloat(): float Value
    symbol_t getFloat;
    std::string getFloatName = "GETFLOAT";
    getFloatName += '\0';
    getFloat.type = ST_PROCEDURE;
    getFloat.variable_type.type = T_RW_FLOAT;
    this->AddGlobalSymbol(getFloatName,getFloat);

    // getString(): string Value
    symbol_t getString;
    std::string getStringName = "GETSTRING";
    getStringName += '\0';
    getString.type = ST_PROCEDURE;
    getString.variable_type.type = T_RW_STRING;
    this->AddGlobalSymbol(getStringName,getString);
    
}

void Scope::PushScope(std::string name)
{
    std::string new_scope_name = this->current_scope_name + name;
    symbol_table_t temp;
    std::list<std::string> code_list; //cg Code Generation

    this->symbol_tables[new_scope_name] = temp;
    this->local_code_map[new_scope_name] = code_list; //cg

    this->current_procedure_name = name;
    this->current_scope_name = new_scope_name;

    this->procedure_name_stack.push_back(name);
    this->scope_stack.push_back(new_scope_name);

    this->scope_reg_ct_stack.push_back(this->reg_ct_local); //cg
    this->reg_ct_local = 0; //cg
}

void Scope::PopScope()
{
    this->scope_stack.pop_back();
    this->current_scope_name = this->scope_stack.back();
    this->procedure_name_stack.pop_back();
    this->current_procedure_name = this->procedure_name_stack.back();

    this->reg_ct_local = this->scope_reg_ct_stack.back(); //cg
    this->scope_reg_ct_stack.pop_back(); //cg
}

std::string Scope::getProcedureName()
{
    if (this->procedure_name_stack.front() == this->procedure_name_stack.back()){
        return "";
    }else{
        return this->current_procedure_name;
    }
}

void Scope::AddSymbol(std::string name, symbol_t symbol)
{
    std::string scope_tag = this->current_scope_name;
    this->symbol_tables[scope_tag][name] = symbol;
}

void Scope::AddGlobalSymbol(std::string name, symbol_t symbol)
{
    symbol.variable_type._is_global=true;
    this->global_symbol_table[name] = symbol;
}

std::map<std::string,symbol_t>::iterator Scope::FindLocal(std::string name, bool* success)
{
    std::map<std::string,symbol_t>::iterator ret;
    ret = this->symbol_tables[this->current_scope_name].find(name);
    *success = !(ret == this->symbol_tables[this->current_scope_name].end());
    return ret;
}

std::map<std::string,symbol_t>::iterator Scope::FindGlobal(std::string name, bool* success)
{
    std::map<std::string,symbol_t>::iterator ret;
    ret = this->global_symbol_table.find(name);
    *success = !(ret == this->global_symbol_table.end());
    return ret;
}

std::map<std::string,symbol_t>::iterator Scope::Find(std::string name, bool* success){
    std::map<std::string,symbol_t>::iterator ret;
    ret = this->FindLocal(name, success);

    if (*success == false){
        ret = this->FindGlobal(name, success);
    }
    return ret;
}

void Scope::writeCode(std::string line, bool global /*= false*/)
{
    if (global) {
        this->global_code.push_back(line);
    } else {
        this->local_code_map[this->current_scope_name].push_back(line);
    }
    
}

void Scope::PrintScope()
{
    // Print Global Scope
    std:: cout << "Global Scope:" << std::endl;
    symbol_table_t::iterator it;
    for (it = this->global_symbol_table.begin(); it != this->global_symbol_table.end(); it++)
    {
        std::cout << "\t" << it->first << " ";
        if (it->second.type == ST_VARIABLE ||
            it->second.type == ST_PROCEDURE){

            std::cout << token_type_to_string(it->second.variable_type.type);

            if (it->second.type == ST_PROCEDURE){
                // Print parameter types
                for (unsigned int i=0; i<it->second.parameter_ct; i++)
                {
                    std::cout << token_type_to_string(it->second.parameter_type_arr[i].type);
                }
            }
        }
        std::cout << std::endl;
    }

    // Print Local Scopes
    std::map<std::string, symbol_table_t>::iterator itm;
    for (itm = this->symbol_tables.begin(); itm != this->symbol_tables.end(); itm++)
    {
        std::cout << "Local Scope: " << itm->first << std::endl;

        symbol_table_t::iterator it;
        for (it = itm->second.begin(); it != itm->second.end(); it++)
        {
            std::cout << "\t" << it->first << " ";
            if (it->second.type == ST_VARIABLE ||
                it->second.type == ST_PROCEDURE){

                std::cout << token_type_to_string(it->second.variable_type.type);

                if (it->second.type == ST_PROCEDURE){
                    // Print parameter types
                    for (unsigned int i=0; i<it->second.parameter_ct; i++)
                    {

                        std::cout << token_type_to_string(it->second.parameter_type_arr[i].type);                        
                        std::cout << " ";
                    }
                }
            }
            std::cout << std::endl;
        }
    }
}

void PrintList(std::list<std::string> list)
{
    std::list<std::string>::iterator it;
    for (it = list.begin(); it != list.end(); it++){
        std::cout << *it << std::endl;
    }
}

void Scope::PrintCode(){
    std::cout << ";******************** LLVM Assembly ********************" << std::endl;
    PrintList(this->global_code);

    std::map<std::string, std::list<std::string>>::iterator it;
    for (it = this->local_code_map.begin(); it != this->local_code_map.end(); it++){
        std::cout << ";******************** " << it->first << " ********************" << std::endl;
        PrintList(it->second);
    }
}