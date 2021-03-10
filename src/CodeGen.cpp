#include "Parser.hpp"
#include <sstream>

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
        }else{
                unsigned int d = this->scope->reg_ct_local;
                ss << "  %" << d <<  " = alloca "; //  %d = alloca 
                if ( symbol->variable_type.is_array == false){
                        switch (symbol->variable_type.type){
                        case T_RW_INTEGER:
                                // %d = alloca i32, align 4
                                ss << "i32, align 4";
                                break;
                        case T_RW_FLOAT:
                                // %d = alloca float, align 4
                                ss << "float, align 4";
                                break;
                        case T_RW_BOOL:
                                // %d = alloca i8
                                ss << "i8";
                                break;
                        case T_RW_STRING:
                                // %d = alloca i8*, align 8
                                ss << "i8*, align 8";
                                break;
                        default:
                                break;
                        }
                } else {
                        unsigned int len = symbol->parameter_ct;
                        switch (symbol->variable_type.type){
                        case T_RW_INTEGER:
                                // %d = alloca [len x i32], align 4*len
                                ss << "[" << len << " x i32], align " << 4*len;
                                break;
                        case T_RW_FLOAT:
                                // %d = alloca [len X float], align 4*len
                                ss << "[" << len << " x float], align " << 4*len;
                                break;
                        case T_RW_BOOL:
                                // %d = alloca [len x i8]
                                ss << "[" << len << " x i8]";
                                break;
                        case T_RW_STRING:
                                // %d = alloca [len x i8*], align 8*len
                                ss << "[" << len << " x i8*], align " << 8*len;
                                break;
                        default:
                                break;
                        }
                }
                symbol->variable_type.reg_ct = this->scope->reg_ct_local;
                this->scope->reg_ct_local++;
        }

        str = ss.str();
        this->scope->writeCode(str, global);

}

std::string get_llvm_type(type_holder_t type)
{
        switch (type.type){
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

void Parser::genAssignmentStatement(type_holder_t dest_type, type_holder_t expr_type)
{
        std::ostringstream ss;
        std::string str;

        std::string dtype_str = get_llvm_type(dest_type);

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
                        // Both are the same
                        // store <type> %expression, <type>* %destination
                        ss << "  store " << dtype_str << " %" << expr_type.reg_ct << ", " << dtype_str << "* %" << dest_type.reg_ct;
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

        str = ss.str();
        this->scope->writeCode(str);
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

void Parser::genConstant(std::list<token_t>::iterator itr, bool is_negative /*= false*/)
{
        std::ostringstream ss0;
        std::ostringstream ss1;
        std::string init;
        std::string store;

        std::string temp;

        unsigned int g = this->scope->reg_ct_global;
        unsigned int d = this->scope->reg_ct_local;

        switch (itr->type){
        case T_RW_TRUE:
                ss0 << "  %" << d << " = alloca i8";
                ss1 << "  store i8 1, i8* %" << d;
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());
                break;
        case T_RW_FALSE:
                ss0 << "  %" << d << " = alloca i8";
                ss1 << "  store i8 1, i8* %" << d;
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());
                break;
        case T_CONST_STRING:
                temp = *(itr->getStringValue());
                ss0 << "@" << g << " = constant [" << temp.length()+1 << " x i8] c\"" << temp << "\\00\"";
                ss1 << "  %" << d << " = getelementptr [" << temp.length()+1 << " x i8], [" << temp.length()+1 << " x i8]* @" << g << ", i64 0, i64 0";
                this->scope->reg_ct_global++;
                this->scope->writeCode(ss0.str(), true);
                this->scope->writeCode(ss1.str());
                break;
        case T_CONST_INTEGER:
                ss0 << "  %" << d << " = alloca i32, align 4";
                ss1 << "  store i32 " << (-1*is_negative)*itr->getIntValue() << ", i32* %" << d;
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());
                break;
        case T_CONST_FLOAT:
                ss0 << "  %" << d << " = alloca float, align 4";
                ss1 << "  store float " << (-1*is_negative)*itr->getFloatValue() << ", float* %" << d;
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());
                break;
        default:
                break;
        }
        
        this->scope->reg_ct_local++;

}