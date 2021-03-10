#include "Parser.hpp"
#include <sstream>

std::string get_llvm_type(token_type_e type)
{
        switch (type){
        case T_RW_INTEGER:
                return "i32";
        case T_RW_FLOAT:
                return "float";
        case T_RW_BOOL:
                return "i8";
        case T_RW_STRING:
                return "i8*";
        default:
                return "";
        }
}

unsigned int get_llvm_align(token_type_e type)
{
        switch (type){
        case T_RW_INTEGER:
                return 4;
        case T_RW_FLOAT:
                return 4;
        case T_RW_BOOL:
                return 1;
        case T_RW_STRING:
                return 8;
        default:
                return 0;
        }
}

unsigned int Parser::genAlloca(token_type_e type, bool is_arr /*= false*/, unsigned int len /*= 0*/)
{
        std::ostringstream ss;
        std::string align_str;
        std::string type_str = get_llvm_type(type);
        unsigned int size = get_llvm_align(type);

        unsigned int align_int = is_arr ? size*len : size;
        if (align_int > 1){
                std::ostringstream tempss;
                tempss << ", align " << align_int;
                align_str = tempss.str();
        }else{
                align_str = "";
        }

        unsigned int d = this->scope->reg_ct_local;
        if (is_arr){
                // %d = alloca [len x i32], align tyes_size*len
                ss << "  %" << d <<  " = alloca [" << len << " x " << type_str << align_str;
        } else {                
                ss << "  %" << d <<  " = alloca " << type_str << align_str;
        }
        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

unsigned int Parser::genLoadReg(token_type_e type, unsigned int location_reg)
{
        // %d = load <type>, <type>* %location_reg
        std::ostringstream ss;
        std::string type_str = get_llvm_type(type);
        unsigned int d = this->scope->reg_ct_local;

        ss << "  %" << d << " = load " << type_str << ", " << type_str << "* %" << location_reg;

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

void Parser::genStoreReg(token_type_e type, unsigned int expr_reg, unsigned int dest_reg)
{
        // store <type> %expression, <type>* %destination
        std::ostringstream ss;
        std::string type_str = get_llvm_type(type);

        ss << "  store " << type_str << " %" << expr_reg << ", " << type_str << "* %" << dest_reg;

        this->scope->writeCode(ss.str());
}

void Parser::genStoreConst(unsigned int dest_reg, bool value){
        // store i8 value, i8* %destination
        std::ostringstream ss;
        std::string type_str = get_llvm_type(T_RW_BOOL);

        ss << "  store " << type_str << " " << value << ", " << type_str << "* %" << dest_reg;

        this->scope->writeCode(ss.str());
}

void Parser::genStoreConst(unsigned int dest_reg, int value)
{
        // store i32 value, i32* %destination
        std::ostringstream ss;
        std::string type_str = get_llvm_type(T_RW_INTEGER);

        ss << "  store " << type_str << " " << value << ", " << type_str << "* %" << dest_reg;

        this->scope->writeCode(ss.str());
}

void Parser::genStoreConst(unsigned int dest_reg, float value)
{
        // store float value, float* %destination
        std::ostringstream ss;
        std::string type_str = get_llvm_type(T_RW_FLOAT);

        ss << "  store " << type_str << " " << value << ", " << type_str << "* %" << dest_reg;

        this->scope->writeCode(ss.str());
}

void Parser::genVariableDeclaration( symbol_t* symbol, bool global)
{
        std::ostringstream ss;
        std::string str;

        // How to track local/global register usage
        if (global){
                if ( symbol->variable_type.is_array == false){
                        ss << "@" << this->scope->reg_ct_global << " "; // @g
                        switch (symbol->variable_type.type){
                        case T_RW_INTEGER:
                                // @g = global i32 0, align 4
                                ss << "= global i32 0, align 4";
                                break;
                        case T_RW_FLOAT:
                                // @g = global float 0, align 4
                                ss << "= global float 0, align 4";
                                break;
                        case T_RW_BOOL:
                                // @g = global i8 0
                                ss << "= global i8 0";
                                break;
                        case T_RW_STRING:
                                // @g = global i8* null, align 8
                                ss << "= global i8* null, align 8";
                                break;
                        default:
                                break;
                        }
                } else {
                        unsigned int len = symbol->parameter_ct;
                        ss << "@" << this->scope->reg_ct_global << " = global [" 
                        << len << " x "; // @g = global [len x 
                        switch (symbol->variable_type.type){
                        case T_RW_INTEGER:
                                // @g = global [len x i32] zeroinitializer, align 4*len
                                ss << "i32] zeroinitializer, align " << 4*len;
                                break;
                        case T_RW_FLOAT:
                                // @g = global [len X float] zeroinitializer, align 4*len
                                ss << "float] zeroinitializer, align " << 4*len;
                                break;
                        case T_RW_BOOL:
                                // @g = global [len x i8] zeroinitializer
                                ss << "i8] zeroinitializer";
                                break;
                        case T_RW_STRING:
                                // @g = global [len x i8*] zeroinitializer, align 8*len
                                ss << "i8*] zeroinitializer, align " << 8*len;
                                break;
                        default:
                                break;
                        }
                }
                symbol->variable_type.reg_ct = this->scope->reg_ct_global;
                this->scope->reg_ct_global++;
                str = ss.str();
                this->scope->writeCode(str, global);
        }else{
                symbol->variable_type.reg_ct = this->genAlloca(symbol->variable_type.type, 
                                symbol->variable_type.is_array, symbol->variable_type.array_length);
        }
}



void Parser::genAssignmentStatement(type_holder_t dest_type, type_holder_t expr_type)
{
        std::ostringstream ss0;
        std::ostringstream ss1;

        std::string dtype_str = get_llvm_type(dest_type.type);

        unsigned int d = this->scope->reg_ct_local;

        if (dest_type._is_global){
                // TODO Implement GEP
                // update dest_type.reg_ct
        }

        if (expr_type._is_global){
                // TODO Implement GEP
                // update expr_type.reg_ct
        }

        if (dest_type.is_array){
                // TODO Implement memcpy
        } else {
                if (type_holder_cmp(dest_type, expr_type)){
                        // TODO This does not work for string constants
                        // Both are the same
                        // store <type> %expression, <type>* %destination
                        genStoreReg(dest_type.type, d-1, dest_type.reg_ct);                        
                } else if ( ((expr_type.type == T_RW_INTEGER) || (expr_type.type == T_RW_FLOAT)) &&
                        ((dest_type.type == T_RW_INTEGER) || (dest_type.type == T_RW_FLOAT))) {
                        // Combinations of int and float are allowed
                        // TODO Typecase to destination
                } else if ( ((expr_type.type == T_RW_INTEGER) || (expr_type.type == T_RW_BOOL)) &&
                        ((dest_type.type == T_RW_INTEGER) || (dest_type.type == T_RW_BOOL))) {
                        // Combinations of int and bool are allowed
                        // TODO Typecast to destination
                }
        }
        
}

void Parser::genProgramHeader()
{
        this->scope->writeCode("define i32 @main() {");
}

void Parser::genProgramBodyEnd()
{
        this->scope->writeCode("  ret i32 0");
        this->scope->writeCode("}");
}

void Parser::genConstant(std::list<token_t>::iterator itr, type_holder_t* parameter_type, bool is_negative /*= false*/)
{
        std::ostringstream ss0;
        std::ostringstream ss1;
        std::ostringstream ss2;
        std::string init;
        std::string store;

        std::string temp;
        

        unsigned int g = this->scope->reg_ct_global;
        unsigned int d = this->scope->reg_ct_local;
        unsigned int temp_reg = 0;

        switch (itr->type){
        case T_RW_TRUE:
                temp_reg = this->genAlloca(T_RW_BOOL);
                this->genStoreConst(temp_reg, true);
                parameter_type->reg_ct = this->genLoadReg(T_RW_BOOL, temp_reg);
                break;
        case T_RW_FALSE:
                temp_reg = this->genAlloca(T_RW_BOOL);
                this->genStoreConst(temp_reg, false);
                parameter_type->reg_ct = this->genLoadReg(T_RW_BOOL, temp_reg);
                break;
        case T_CONST_STRING: // TODO update reg_ct
                temp = *(itr->getStringValue());
                ss0 << "@" << g << " = constant [" << temp.length()+1 << " x i8] c\"" << temp << "\\00\"";
                ss1 << "  %" << d << " = getelementptr [" << temp.length()+1 << " x i8], [" << temp.length()+1 << " x i8]* @" << g << ", i64 0, i64 0";
                this->scope->reg_ct_global++;
                this->scope->writeCode(ss0.str(), true);
                this->scope->writeCode(ss1.str());
                this->scope->reg_ct_local++;
                break;
        case T_CONST_INTEGER: // TODO update reg_ct
                ss0 << "  %" << d << " = alloca i32, align 4";
                ss1 << "  store i32 " << (-1*is_negative)*itr->getIntValue() << ", i32* %" << d;
                ss2 << "  %" << d+1 << " = load i32, i32* %" << d << ", align 4";
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());
                this->scope->writeCode(ss2.str());
                this->scope->reg_ct_local++;
                this->scope->reg_ct_local++;
                break;
        case T_CONST_FLOAT: // TODO update reg_ct
                ss0 << "  %" << d << " = alloca float, align 4";
                ss1 << "  store float " << (-1*is_negative)*itr->getFloatValue() << ", float* %" << d;
                ss2 << "  %" << d+1 << " = load float, float* %" << d << ", align 4";
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());
                this->scope->writeCode(ss2.str());
                this->scope->reg_ct_local++;
                this->scope->reg_ct_local++;
                break;
        default:
                break;
        }
        
        

}