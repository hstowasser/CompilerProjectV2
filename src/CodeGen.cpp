#include "Parser.hpp"
#include <sstream>
#include <cstring>

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

unsigned int get_llvm_size(token_type_e type)
{
        unsigned int ret;
        switch (type){
        case T_RW_INTEGER:
                ret = 4;
                break;
        case T_RW_FLOAT:
                ret = 4;
                break;
        case T_RW_BOOL:
                ret = 1;
                break;
        case T_RW_STRING:
                ret = 8;
                break;
        default:
                return 0;
        }
        return ret;
}

unsigned int get_llvm_align(token_type_e type, bool is_arr, unsigned int len)
{
        unsigned int ret = get_llvm_size(type);

        if (is_arr){
                ret = ret * len;
                if (ret <=0 ){
                        ret = 1;
                } else if (ret < 4) {
                        ret = 4;
                } else if (ret < 8) {
                        ret = 8;
                } else if (ret > 8) {
                        ret = 16;
                }
        }
        return ret;
}

std::string getHex(float x)
{
        std::ostringstream ss;
        constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        double holder = x; // LLVM IR uses double percision constants
        unsigned long temp;
        memcpy(&temp,&holder, sizeof(double));
        ss << "0x";
        for ( int i = 15; i >= 0; i-- ){
                ss << hexmap[(temp >> i*4) & 0xF];
        }
        return ss.str();
}

unsigned int Parser::genAlloca(token_type_e type, bool is_arr /*= false*/, unsigned int len /*= 0*/)
{
        std::ostringstream ss;
        std::string align_str;
        std::string type_str = get_llvm_type(type);
        unsigned int align_int = get_llvm_align(type, is_arr, len);

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
                ss << "  %" << d <<  " = alloca [" << len << " x " << type_str << "]" << align_str;
        } else {                
                ss << "  %" << d <<  " = alloca " << type_str << align_str;
        }
        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;

        if (is_arr){
                // TODO zero initialize arrays
        } else {
                switch (type)
                {
                case T_RW_INTEGER:
                        this->genStoreConst(d, (int)0);
                        break;
                case T_RW_FLOAT:
                        this->genStoreConst(d, (float)0);
                        break;
                case T_RW_BOOL:
                        this->genStoreConst(d, (bool)0);
                        break;
                case T_RW_STRING:
                        // TODO or Not needed?
                        break;
                default:
                        break;
                }
        }
        return d;
}

unsigned int Parser::genLoadReg(token_type_e type, unsigned int location_reg, bool global /*= false*/)
{
        // %d = load <type>, <type>* %location_reg
        std::ostringstream ss;
        std::string type_str = get_llvm_type(type);
        unsigned int d = this->scope->reg_ct_local;

        ss << "  %" << d << " = load " << type_str << ", " << type_str << "* ";
        if (global){
                ss << "@" << location_reg;
        } else {
                ss << "%" << location_reg;
        }
        

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

void Parser::genStoreReg(token_type_e type, unsigned int expr_reg, unsigned int dest_reg, bool global /*= false*/)
{
        // store <type> %expression, <type>* %destination
        std::ostringstream ss;
        std::string type_str = get_llvm_type(type);

        ss << "  store " << type_str << " %" << expr_reg << ", " << type_str << "* ";
        if (global){
                ss << "@" << dest_reg;
        } else {
                ss << "%" << dest_reg;
        }

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

        ss << "  store " << type_str << " " << getHex(value) << ", " << type_str << "* %" << dest_reg;

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
                                ss << "= global float 0.0, align 4";
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
                        unsigned int len = symbol->variable_type.array_length;
                        std::string type_str = get_llvm_type(symbol->variable_type.type);
                        unsigned int align_int = get_llvm_align(symbol->variable_type.type, symbol->variable_type.is_array, len);
                        ss << "@" << this->scope->reg_ct_global << " = global [" 
                        << len << " x "; // @g = global [len x 
                        ss << type_str << "] zeroinitializer, align " << align_int;
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

        std::string dtype_str = get_llvm_type(dest_type.type);

        //unsigned int d = this->scope->reg_ct_local;

        if (dest_type.is_array){
                if (type_holder_cmp(dest_type, expr_type)){
                        std::ostringstream ss;
                        std::ostringstream ss0;
                        std::ostringstream ss1;
                        unsigned int size = get_llvm_size(dest_type.type);
                        unsigned int len = dest_type.array_length * size;

                        // GEP in parseName was removed so will need to add a GEP here.
                        unsigned int reg_a = this->genGEP_Head(dest_type, dest_type._is_global);
                        unsigned int reg_b = this->genGEP_Head(expr_type, expr_type._is_global);

                        // bitcast both to *8
                        //%8 = bitcast i32* %7 to i8*
                        ss0 << "  %" << this->scope->reg_ct_local <<" = bitcast " << dtype_str << "* %" << reg_a << " to i8*";
                        reg_a = this->scope->reg_ct_local;
                        this->scope->reg_ct_local++;

                        ss1 << "  %" << this->scope->reg_ct_local <<" = bitcast " << dtype_str << "* %" << reg_b << " to i8*";
                        reg_b = this->scope->reg_ct_local;
                        this->scope->reg_ct_local++;

                        ss << "  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align " << size << " %" << reg_a << ", i8* align " 
                                << size << " %" << reg_b << ", i64 " << len << ", i1 false)";
                        
                        this->scope->writeCode(ss0.str());
                        this->scope->writeCode(ss1.str());
                        this->scope->writeCode(ss.str());
                } else {
                        // Should be handled by loop wrapper
                }
        } else {
                if (type_holder_cmp(dest_type, expr_type)){
                        // Both are the same
                        // store <type> %expression, <type>* %destination
                        //genStoreReg(dest_type.type, d-1, dest_type.reg_ct, dest_type._is_global);
                        genStoreReg(dest_type.type, expr_type.reg_ct, dest_type.reg_ct, dest_type._is_global);                     
                } else if ( ((expr_type.type == T_RW_INTEGER) || (expr_type.type == T_RW_FLOAT)) &&
                        ((dest_type.type == T_RW_INTEGER) || (dest_type.type == T_RW_FLOAT))) {
                        // Combinations of int and float are allowed
                        // Typecase to destination
                        if ( dest_type.type == T_RW_INTEGER){
                                expr_type.reg_ct = this->genFloatToInt(expr_type.reg_ct);
                                expr_type.type = T_RW_INTEGER;
                        } else {
                                expr_type.reg_ct = this->genIntToFloat(expr_type.reg_ct);
                                expr_type.type = T_RW_FLOAT;
                        }
                        genStoreReg(dest_type.type, expr_type.reg_ct, dest_type.reg_ct, dest_type._is_global);
                } else if ( ((expr_type.type == T_RW_INTEGER) || (expr_type.type == T_RW_BOOL)) &&
                        ((dest_type.type == T_RW_INTEGER) || (dest_type.type == T_RW_BOOL))) {
                        // Combinations of int and bool are allowed
                        // Typecast to destination
                        if ( dest_type.type == T_RW_INTEGER){
                                expr_type.reg_ct = this->genBoolToInt(expr_type.reg_ct);
                                expr_type.type = T_RW_INTEGER;
                        } else {
                                expr_type.reg_ct = this->genIntToBool(expr_type.reg_ct);
                                expr_type.type = T_RW_BOOL;
                        }
                        genStoreReg(dest_type.type, expr_type.reg_ct, dest_type.reg_ct, dest_type._is_global);
                }
        }
        
}

unsigned int Parser::genTypeConversion(token_type_e desired_type, type_holder_t parameter_type)
{
        unsigned int reg = parameter_type.reg_ct;
        switch (desired_type)
        {
        case T_RW_BOOL:
                if (parameter_type.type == T_RW_INTEGER){
                        reg = this->genIntToBool(parameter_type.reg_ct);
                }
                break;
        case T_RW_INTEGER:
                if (parameter_type.type == T_RW_FLOAT){
                        reg = this->genIntToBool(parameter_type.reg_ct);
                }else if (parameter_type.type == T_RW_BOOL){
                        reg = this->genBoolToInt(parameter_type.reg_ct);
                }
                break;
        case T_RW_FLOAT:
                if (parameter_type.type == T_RW_INTEGER){
                        reg = this->genIntToFloat(parameter_type.reg_ct);
                }
                break;
        default:
                break;
        }
        return reg;
}

void Parser::genProgramHeader()
{
        this->scope->writeCode("define i32 @main() {");
        this->scope->reg_ct_local++; // Not sure why, but there is always an unused reg after parameter list
}

void Parser::genProgramBodyEnd()
{
        this->scope->writeCode("  ret i32 0");
        this->scope->writeCode("}");
}

void Parser::genProcedureHeader(symbol_t symbol, std::string name)
{
        name.pop_back(); // remove Duplicate null terminator
        std::ostringstream ss;
        if ( symbol.variable_type.is_array){
                // I don't think this is supported
        } else {
                ss << "define " << get_llvm_type(symbol.variable_type.type) <<" @" << name << symbol._procedure_ct << "(";
        }

        // Define parameters
        for (unsigned int i = 0; i < symbol.parameter_ct; i++){
                if (i != 0){
                        ss << ", ";
                }
                if (symbol.parameter_type_arr[i].is_array){
                        unsigned int len = symbol.parameter_type_arr[i].array_length;
                        ss << "[" << len << " x " << get_llvm_type(symbol.parameter_type_arr[i].type) << "]* %" <<  symbol.parameter_type_arr[i].reg_ct;
                } else {
                        ss << get_llvm_type(symbol.parameter_type_arr[i].type) << " %" << symbol.parameter_type_arr[i].reg_ct;
                }                
                
        }
        this->scope->reg_ct_local++; // Not sure why, but there is always an unused reg after parameter list
        
        ss << ") {";
        this->scope->writeCode(ss.str());
        
}

void Parser::genProcedureEnd(type_holder_t return_type)
{
        if (return_type.is_array == false){
                switch (return_type.type)
                {
                case T_RW_INTEGER:
                        this->scope->writeCode("  ret i32 0");
                        break;
                case T_RW_FLOAT:
                        this->scope->writeCode("  ret float 0.0");
                        break;
                case T_RW_BOOL:
                        this->scope->writeCode("  ret i8 0");
                        break;
                case T_RW_STRING:
                        this->scope->writeCode("  ret i8* null");
                        break;
                default:
                        break;
                }
        } else {
                // Array returns not supported
        }
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
        case T_CONST_STRING:
                temp = *(itr->getStringValue());
                ss0 << "@" << g << " = constant [" << temp.length()+1 << " x i8] c\"" << temp << "\\00\"";
                this->scope->writeCode(ss0.str(), true);
                this->scope->reg_ct_global++;
                ss1 << "  %" << d << " = getelementptr [" << temp.length()+1 << " x i8], [" << temp.length()+1 << " x i8]* @" << g << ", i64 0, i64 0";
                parameter_type->reg_ct = d;             
                this->scope->writeCode(ss1.str());
                this->scope->reg_ct_local++;
                break;
        case T_CONST_INTEGER:
                temp_reg = this->genAlloca(T_RW_INTEGER);
                this->genStoreConst(temp_reg, (1-2*is_negative)*itr->getIntValue());
                parameter_type->reg_ct = this->genLoadReg(T_RW_INTEGER, temp_reg);
                break;
        case T_CONST_FLOAT:
                temp_reg = this->genAlloca(T_RW_FLOAT);
                this->genStoreConst(temp_reg, (1-2*is_negative)*itr->getFloatValue());
                parameter_type->reg_ct = this->genLoadReg(T_RW_FLOAT, temp_reg);
                break;
        default:
                break;
        }
}

unsigned int Parser::genProcedureCall(symbol_t symbol, std::string name, std::list<unsigned int> regs)
{
        std::ostringstream ss;
        unsigned int d = this->scope->reg_ct_local;
        name.pop_back(); // remove Duplicate null terminator

        ss << "  %" << d << " = call " << get_llvm_type(symbol.variable_type.type) << " @" << name << symbol._procedure_ct << "(";

        auto it = regs.begin();
        for (unsigned int i = 0; i < symbol.parameter_ct; i++){
                if (i != 0){
                        ss << ", ";
                }
                
                if (symbol.parameter_type_arr[i].is_array){
                        unsigned int len = symbol.parameter_type_arr[i].array_length;
                        ss << "[" << len << " x " << get_llvm_type(symbol.parameter_type_arr[i].type) << "]* %" <<  *it;
                } else {
                        ss << get_llvm_type(symbol.parameter_type_arr[i].type) << " %" << *it;
                } 
                it++;
        }
        ss << ")";
        
        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

unsigned int Parser::genIntToLong(unsigned int reg)
{
        std::ostringstream ss;
        unsigned int d = this->scope->reg_ct_local;

        ss << "  %" << d << " = sext i32 %" << reg << " to i64";

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

unsigned int Parser::genGEP(type_holder_t parameter_type, unsigned int index_reg, bool global /*= false*/)
{
        std::ostringstream ss;
        unsigned int d = this->scope->reg_ct_local;

        unsigned int len = parameter_type.array_length;
        std::string type_str = get_llvm_type(parameter_type.type);

        if (global){
                ss << "  %" << d << " = getelementptr inbounds [" << len << " x " << type_str << "], [" 
                        << len << " x " << type_str << "]* @" << parameter_type.reg_ct << ", i64 0, i64 %" << index_reg;
        } else {
                ss << "  %" << d << " = getelementptr inbounds [" << len << " x " << type_str << "], [" 
                        << len << " x " << type_str << "]* %" << parameter_type.reg_ct << ", i64 0, i64 %" << index_reg;
        }

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

unsigned int Parser::genGEP_Head(type_holder_t parameter_type,  bool global /*= false*/)
{
        std::ostringstream ss;
        unsigned int d = this->scope->reg_ct_local;

        unsigned int len = parameter_type.array_length;
        std::string type_str = get_llvm_type(parameter_type.type);

        if (global){
                ss << "  %" << d << " = getelementptr inbounds [" << len << " x " << type_str << "], [" 
                        << len << " x " << type_str << "]* @" << parameter_type.reg_ct << ", i64 0, i64 0";
        } else {
                ss << "  %" << d << " = getelementptr inbounds [" << len << " x " << type_str << "], [" 
                        << len << " x " << type_str << "]* %" << parameter_type.reg_ct << ", i64 0, i64 0";
        }

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}




unsigned int Parser::genTerm(token_type_e op, token_type_e type_a, unsigned int reg_a, token_type_e type_b, unsigned int reg_b)
{
        std::ostringstream ss;
        unsigned int d;

        if ( (type_a == T_RW_FLOAT) || (type_b == T_RW_FLOAT)) {
                if ( type_a == T_RW_INTEGER){
                        // convert to float
                        reg_a = genIntToFloat(reg_a);
                } else if (type_b == T_RW_INTEGER) {
                        // convert to float
                        reg_b = genIntToFloat(reg_b);
                }
                d = this->scope->reg_ct_local++;

                if ( op == T_OP_TERM_DIVIDE){
                        ss << "  %" << d << " = fdiv float %" << reg_a << ", %" << reg_b;

                } else { //T_OP_TERM_MULTIPLY
                        ss << "  %" << d << " = fmul float %" << reg_a << ", %" << reg_b;
                }
        } else { // Both are integers
                d = this->scope->reg_ct_local++;
                if ( op == T_OP_TERM_DIVIDE){
                        ss << "  %" << d << " = sdiv i32 %" << reg_a << ", %" << reg_b;
                } else { //T_OP_TERM_MULTIPLY
                        ss << "  %" << d << " = mul nsw i32 %" << reg_a << ", %" << reg_b;
                }
        }        

        this->scope->writeCode(ss.str());
        return d;
}

unsigned int Parser::genRelation(token_type_e op, token_type_e type_a, unsigned int reg_a, token_type_e type_b, unsigned int reg_b)
{
        std::ostringstream ss;
        

        std::string op_str;

        if (type_a == T_RW_BOOL || type_b == T_RW_BOOL){
                std::ostringstream ss0;
                std::ostringstream ss1;
                std::ostringstream ss2;
                std::ostringstream ss3;

                if (type_a == T_RW_BOOL) {
                        ss0 << "  %" << this->scope->reg_ct_local << " = trunc i8 %" << reg_a << " to i1";
                } else {
                        //%5 = icmp ne i32 %4, 0
                        ss0 << "  %" << this->scope->reg_ct_local << " = icmp ne i32 %" << reg_a << ", 0"; // True if greater than zero
                        //ss0 << "  %" << this->scope->reg_ct_local << " = trunc i32 %" << reg_a << " to i1";
                }
                reg_a = this->scope->reg_ct_local;
                this->scope->reg_ct_local++;
                ss1 << "  %" << this->scope->reg_ct_local << " = zext i1 %" << reg_a << "to i32";
                reg_a = this->scope->reg_ct_local;
                this->scope->reg_ct_local++;
                this->scope->writeCode(ss0.str());
                this->scope->writeCode(ss1.str());

                if (type_b == T_RW_BOOL) {
                        ss2 << "  %" << this->scope->reg_ct_local << " = trunc i8 %" << reg_b << " to i1";
                } else {
                        ss2 << "  %" << this->scope->reg_ct_local << " = icmp ne i32 %" << reg_b << ", 0";
                        //ss2 << "  %" << this->scope->reg_ct_local << " = trunc i32 %" << reg_b << " to i1";
                }                
                reg_b = this->scope->reg_ct_local;
                this->scope->reg_ct_local++;
                ss3 << "  %" << this->scope->reg_ct_local << " = zext i1 %" << reg_b << "to i32";
                reg_b = this->scope->reg_ct_local;
                this->scope->reg_ct_local++;
                this->scope->writeCode(ss2.str());
                this->scope->writeCode(ss3.str());
        }

        if ( type_a == T_RW_FLOAT){
                switch (op){
                case T_OP_REL_LESS:
                        op_str = " = fcmp olt float %";
                        break;
                case T_OP_REL_GREATER_EQUAL:
                        op_str = " = fcmp oge float %";
                        break;
                case T_OP_REL_LESS_EQUAL:
                        op_str = " = fcmp ole float %";
                        break;
                case T_OP_REL_GREATER:
                        op_str = " = fcmp ogt float %";
                        break;
                case T_OP_REL_EQUAL:
                        op_str = " = fcmp oeq float %";
                        break;
                case T_OP_REL_NOT_EQUAL:
                        op_str = " = fcmp une float %";
                        break;
                default:
                        break;
                }
        } else { //if (type_a == T_RW_INTEGER){
                switch (op){
                case T_OP_REL_LESS:
                        op_str = " = icmp slt i32 %";
                        break;
                case T_OP_REL_GREATER_EQUAL:
                        op_str = " = icmp sge i32 %";
                        break;
                case T_OP_REL_LESS_EQUAL:
                        op_str = " = icmp sle i32 %";
                        break;
                case T_OP_REL_GREATER:
                        op_str = " = icmp sgt i32 %";
                        break;
                case T_OP_REL_EQUAL:
                        op_str = " = icmp eq i32 %";
                        break;
                case T_OP_REL_NOT_EQUAL:
                        op_str = " = icmp ne i32 %";
                        break;
                default:
                        break;
                }
        }

        ss << "  %" << this->scope->reg_ct_local << op_str << reg_a << ", %" << reg_b;
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss.str());

        unsigned int d = this->scope->reg_ct_local;
        std::ostringstream ss4;
        ss4 << "  %" << this->scope->reg_ct_local << " = zext i1 %" << d-1 << " to i8"; //%8 = zext i1 %7 to i8
        d = this->scope->reg_ct_local;
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss4.str());
        
        return d;
}

unsigned int Parser::genRelationStrings(token_type_e op, unsigned int reg_a, unsigned int reg_b)
{
        std::ostringstream ss0;
        std::ostringstream ss1;
        std::ostringstream ss2;
        unsigned int d = this->scope->reg_ct_local;

        //%7 = call i32 @strcmp(i8* %5, i8* %6)
        ss0 << "  %" << d << " = call i32 @strcmp(i8* %" << reg_a <<", i8* %" << reg_b << ")";
        this->scope->reg_ct_local++;

        if (op == T_OP_REL_EQUAL){
                //%8 = icmp eq i32 %7, 0
                ss1 << "  %" << this->scope->reg_ct_local << " = icmp eq i32 %" << d << ", 0";
        } else { // T_OP_REL_NOT_EQUAL
                ss1 << "  %" << this->scope->reg_ct_local << " = icmp ne i32 %" << d << ", 0";
        }
        d = this->scope->reg_ct_local;
        this->scope->reg_ct_local++;

        ss2 << "  %" << this->scope->reg_ct_local << " = zext i1 %" << d << " to i8"; //%8 = zext i1 %7 to i8
        d = this->scope->reg_ct_local;
        this->scope->reg_ct_local++;

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
        this->scope->writeCode(ss2.str());

        return d;
}

unsigned int Parser::genIntToFloat(unsigned int reg)
{
        // convert to float
        // %6 = sitofp i32 %5 to float
        std::ostringstream ss0;
        unsigned int d = this->scope->reg_ct_local;
        ss0 << "  %" << d << " = sitofp i32 %" << reg << " to float";
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss0.str());
        return d;
}

unsigned int Parser::genFloatToInt(unsigned int reg)
{
        // convert to int
        // %5 = fptosi float %4 to i32
        std::ostringstream ss0;
        unsigned int d = this->scope->reg_ct_local;
        ss0 << "  %" << d << " = fptosi float %" << reg << " to i32";
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss0.str());
        return d;
}

unsigned int Parser::genBoolToInt(unsigned int reg)
{
        // Extend to int
        std::ostringstream ss0;
        std::ostringstream ss1;

        // %5 = trunc i8 %4 to i1
        ss0 << "  %" << this->scope->reg_ct_local << " = trunc i8 %" << reg << " to i1";
        unsigned int temp = this->scope->reg_ct_local;
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss0.str());

        // %6 = zext i1 %5 to i32
        unsigned int d = this->scope->reg_ct_local;
        ss1 << "  %" << d << " = zext i1 %" << temp << " to i32";
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss1.str());

        return d;
}

unsigned int Parser::genIntToBool(unsigned int reg)
{
        // truncate to bool
        std::ostringstream ss0;
        std::ostringstream ss1;

        //%5 = icmp ne i32 %4, 0
        ss0 << "  %" << this->scope->reg_ct_local << " = icmp ne i32 %" << reg << ", 0";
        unsigned int temp = this->scope->reg_ct_local;
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss0.str());

        // %6 = zext i1 %5 to i8
        unsigned int d = this->scope->reg_ct_local;
        ss1 << "  %" << d << " = zext i1 %" << temp << " to i8";
        this->scope->reg_ct_local++;
        this->scope->writeCode(ss1.str());

        return d;
}

unsigned int Parser::genArithOp(token_type_e op, token_type_e type_a, unsigned int reg_a, token_type_e type_b, unsigned int reg_b)
{
        unsigned int d;
        

        if ( (type_a == T_RW_FLOAT) || (type_b == T_RW_FLOAT)) {
                if ( type_a == T_RW_INTEGER){
                        // convert to float
                        reg_a = genIntToFloat(reg_a);
                } else if (type_b == T_RW_INTEGER) {
                        // convert to float
                        reg_b = genIntToFloat(reg_b);
                }
                // Do floating point OP
                std::ostringstream ss;
                d = this->scope->reg_ct_local;

                if (op == T_OP_ARITH_MINUS ) {
                        //%7 = fsub float %5, %6
                        ss << "  %" << d << " = fsub float %" << reg_a << ", %" << reg_b;
                } else { // T_OP_ARITH_PLUS
                        // %7 = fadd float %5, %6
                        ss << "  %" << d << " = fadd float %" << reg_a << ", %" << reg_b;
                }
                this->scope->reg_ct_local++;
                this->scope->writeCode(ss.str());

        } else {
                // Do integer OP
                std::ostringstream ss;
                d = this->scope->reg_ct_local;

                if (op == T_OP_ARITH_MINUS ) {
                        //%7 = sub nsw i32 %5, %6
                        ss << "  %" << d << " = sub nsw i32 %" << reg_a << ", %" << reg_b;
                } else { // T_OP_ARITH_PLUS
                        // %7 = add nsw i32 %5, %6
                        ss << "  %" << d << " = add nsw i32 %" << reg_a << ", %" << reg_b;
                }
                this->scope->reg_ct_local++;
                this->scope->writeCode(ss.str());
        }
        return d;
}

unsigned int Parser::genExpression(token_type_e op, unsigned int reg_a, unsigned int reg_b)
{
        std::ostringstream ss;
        unsigned int d = this->scope->reg_ct_local;

        if ( op == T_OP_BITW_AND) {
                // %7 = and i32 %5, %6
                ss << "  %" << d << " = and i32 %" << reg_a << ", %" << reg_b;
        } else { // T_OP_BITW_OR
                // %7 = or i32 %5, %6
                ss << "  %" << d << " = or i32 %" << reg_a << ", %" << reg_b;
        }

        this->scope->reg_ct_local++;
        this->scope->writeCode(ss.str());
        return d;
}

void Parser::genReturn(token_type_e type, unsigned int reg)
{
        std::ostringstream ss;
        
        ss << "  ret " << get_llvm_type(type) << " %" << reg;

        this->scope->reg_ct_local++; // Another one of these strange invisible registers

        this->scope->writeCode(ss.str());
}

void Parser::genIfHead(unsigned int reg, unsigned int if_label, unsigned int else_label)
{
        std::ostringstream ss0;
        std::ostringstream ss1;
        std::ostringstream ss2;

        unsigned int d = this->scope->reg_ct_local;

        // %5 = trunc i8 %4 to i1
        ss0 << "  %" << d << " = trunc i8 %" << reg << " to i1";
        this->scope->reg_ct_local++;

        // br i1 %5, label %6, label %10
        ss1 << "  br i1 %" << d << ", label %L" << if_label << ", label %L" << else_label;

        ss2 << "L" << if_label << ":";

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
        this->scope->writeCode(ss2.str());
}

void Parser::genIfElse(unsigned int else_label, unsigned int end_label)
{
        std::ostringstream ss0;
        std::ostringstream ss1;

        ss0 << "  br label %L" << end_label;
        ss1 << "L" << else_label << ":";

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
}

void Parser::genIfEnd(unsigned int end_label)
{
        std::ostringstream ss0;
        std::ostringstream ss1;

        ss0 << "  br label %L" << end_label;
        ss1 << "L" << end_label << ":";

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
}


/*      

        Loop assignment operations
**HEAD
*        branch start_label
*Start_label:
        Loop condition operations
**Condition
*        branch body_label or start_label
*Body_label:
        //content

**End
*        branch start_label
*End_label;


*/
void Parser::genLoopHead(unsigned int start_label)
{
        std::ostringstream ss0;
        std::ostringstream ss1;

        ss0 << "  br label %L" << start_label;
        ss1 << "L" << start_label << ":";

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
}

void Parser::genLoopCondition(unsigned int reg, unsigned int body_label, unsigned int end_label, bool dont_tuncate /*= false*/)
{
        std::ostringstream ss0;
        std::ostringstream ss1;
        std::ostringstream ss2;

        unsigned int d = this->scope->reg_ct_local;

        if (dont_tuncate) {
                d = reg; // Evaluate reg directly. Used in Array Operation Loop generation
        } else {
                // %5 = trunc i8 %4 to i1
                ss0 << "  %" << d << " = trunc i8 %" << reg << " to i1";
                this->scope->reg_ct_local++;
        }

        // br i1 %5, label %6, label %10
        ss1 << "  br i1 %" << d << ", label %L" << body_label << ", label %L" << end_label;

        ss2 << "L" << body_label << ":";

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
        this->scope->writeCode(ss2.str());
}

void Parser::genLoopEnd( unsigned int start_label, unsigned int end_label)
{
        std::ostringstream ss0;
        std::ostringstream ss1;

        ss0 << "  br label %L" << start_label;
        ss1 << "L" << end_label << ":";

        this->scope->writeCode(ss0.str());
        this->scope->writeCode(ss1.str());
}

unsigned int Parser::genNegate(token_type_e type, unsigned int reg)
{
        std::ostringstream ss;

        unsigned int d = this->scope->reg_ct_local;

        if (type == T_RW_INTEGER){
                // %5 = sub nsw i32 0, %4
                ss << "  %" << d << " = sub nsw i32 0, %" << reg;
        } else { // T_RW_FLOAT
                // %5 = fneg float %4
                ss << "  %" << d << " = fneg float %" << reg;
        }        

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

unsigned int Parser::genNot(token_type_e type, unsigned int reg)
{
        std::ostringstream ss;

        unsigned int d = this->scope->reg_ct_local;


        ss << "  %" << d << " = xor i32 %" << reg << ", -1";
   

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
        return d;
}

array_op_params Parser::genSetupArrayOp(type_holder_t* type_a, type_holder_t* type_b, token_type_e result_type)
{
        array_op_params ret;

        unsigned int start_label = this->scope->NewLabel();
        unsigned int body_label = this->scope->NewLabel();
        // unsigned int inc_label = this->scope->NewLabel();
        unsigned int end_label = this->scope->NewLabel();

        unsigned int len = type_a->is_array ? type_a->array_length : type_b->array_length;
        //unsigned int size = get_llvm_size(result_type);

        // Allocate i
        std::ostringstream ss_i;
        ret.reg_i = this->scope->reg_ct_local++; // Store location of i
        ss_i << "  %"  << ret.reg_i << " = alloca i64, align 8";
        this->scope->writeCode(ss_i.str());

        // Initialize i
        std::ostringstream ss_ii;
        ss_ii << "store i64 0, i64* %" << ret.reg_i << ", align 8";
        this->scope->writeCode(ss_ii.str());

        // Allocate result array
        ret.ret_arr_type.reg_ct = this->genAlloca(result_type, true, len);

        // genLoopHead
        this->genLoopHead(start_label);

        // Load i
        std::ostringstream ss_il;
        unsigned int temp_i_reg = this->scope->reg_ct_local++; // reg that holds value of i
        ss_il << "  %"  << temp_i_reg << " = load i64, i64* %" << ret.reg_i << ", align 8";
        this->scope->writeCode(ss_il.str());

        // Compare to len
        //%8 = icmp ult i64 %7, 5
        std::ostringstream ss_cmp;
        unsigned int cmp_reg = this->scope->reg_ct_local++;
        ss_cmp << "  %" << cmp_reg << " = icmp ult i64 %" << temp_i_reg << ", " << len;
        this->scope->writeCode(ss_cmp.str());

        // genLoopCondition
        bool dont_truncate = true;
        this->genLoopCondition(cmp_reg, body_label, end_label, dont_truncate);

        // If array load arr[i]
        if (type_a->is_array){
                type_a->reg_ct = this->genGEP(*type_a, temp_i_reg, type_a->_is_global);
                type_a->_is_global = false;
                type_a->is_array = false;
                type_a->reg_ct = this->genLoadReg(type_a->type, type_a->reg_ct);
        }
        if (type_b->is_array){
                type_b->reg_ct = this->genGEP(*type_b, temp_i_reg, type_b->_is_global);
                type_b->_is_global = false;
                type_b->is_array = false;
                type_b->reg_ct = this->genLoadReg(type_b->type, type_b->reg_ct);
        }
        
        
        ret.start_label = start_label;
        // ret.inc_label = inc_label;
        ret.end_label = end_label;
        ret.ret_arr_type.type = result_type;
        ret.ret_arr_type.is_array = true;
        ret.ret_arr_type._is_global = false;
        ret.ret_arr_type.array_length = len;
        return ret;
}


/**
 * needs reg for i
 * needs type and length of return array
 * needs inc_label
 * needs end_label
 * needs start_label
 */
type_holder_t Parser::genEndArrayOp(array_op_params params, unsigned int expr_reg)
{
        // Load i
        std::ostringstream ss_il;
        unsigned int temp_i_reg = this->scope->reg_ct_local++; // reg that holds value of i
        ss_il << "  %"  << temp_i_reg << " = load i64, i64* %" << params.reg_i << ", align 8";
        this->scope->writeCode(ss_il.str());

        // store expr_reg in result array
        unsigned int result_i = genGEP(params.ret_arr_type, temp_i_reg, params.ret_arr_type._is_global);
        this->genStoreReg(params.ret_arr_type.type, expr_reg, result_i, false);


        // Increment i
        std::ostringstream ss_inc;
        unsigned int inc_i_reg = this->scope->reg_ct_local++; // reg that holds value of i++
        ss_inc << "  %"  << inc_i_reg << " = add i64 %" << temp_i_reg<< ", 1";
        this->scope->writeCode(ss_inc.str());
        
        //Store i++ in i
        std::ostringstream ss_store;
        ss_store << "  store i64 %" << inc_i_reg << ", i64* %" << params.reg_i << ", align 8";
        this->scope->writeCode(ss_store.str());

        this->genLoopEnd(params.start_label, params.end_label);
        
        return params.ret_arr_type;
}


array_op_params Parser::genSetupArrayAssign(type_holder_t* type_a, type_holder_t* type_b, token_type_e result_type)
{
        array_op_params ret;

        unsigned int start_label = this->scope->NewLabel();
        unsigned int body_label = this->scope->NewLabel();
        // unsigned int inc_label = this->scope->NewLabel();
        unsigned int end_label = this->scope->NewLabel();

        unsigned int len = type_a->is_array ? type_a->array_length : type_b->array_length;
        //unsigned int size = get_llvm_size(result_type);

        // Allocate i
        std::ostringstream ss_i;
        ret.reg_i = this->scope->reg_ct_local++; // Store location of i
        ss_i << "  %"  << ret.reg_i << " = alloca i64, align 8";
        this->scope->writeCode(ss_i.str());

        // Initialize i
        std::ostringstream ss_ii;
        ss_ii << "store i64 0, i64* %" << ret.reg_i << ", align 8";
        this->scope->writeCode(ss_ii.str());

        // genLoopHead
        this->genLoopHead(start_label);

        // Load i
        std::ostringstream ss_il;
        unsigned int temp_i_reg = this->scope->reg_ct_local++; // reg that holds value of i
        ss_il << "  %"  << temp_i_reg << " = load i64, i64* %" << ret.reg_i << ", align 8";
        this->scope->writeCode(ss_il.str());

        // Compare to len
        //%8 = icmp ult i64 %7, 5
        std::ostringstream ss_cmp;
        unsigned int cmp_reg = this->scope->reg_ct_local++;
        ss_cmp << "  %" << cmp_reg << " = icmp ult i64 %" << temp_i_reg << ", " << len;
        this->scope->writeCode(ss_cmp.str());

        // genLoopCondition
        bool dont_truncate = true;
        this->genLoopCondition(cmp_reg, body_label, end_label, dont_truncate);

        // If array GEP arr[i]
        if (type_a->is_array){
                type_a->reg_ct = this->genGEP(*type_a, temp_i_reg, type_a->_is_global);
                type_a->_is_global = false;
                type_a->is_array = false;
                // Don't load the destination
                //type_a->reg_ct = this->genLoadReg(type_a->type, type_a->reg_ct);
        }
        if (type_b->is_array){
                type_b->reg_ct = this->genGEP(*type_b, temp_i_reg, type_b->_is_global);
                type_b->_is_global = false;
                type_b->is_array = false;
                type_b->reg_ct = this->genLoadReg(type_b->type, type_b->reg_ct);
        }
        
        
        ret.start_label = start_label;
        ret.end_label = end_label;
        return ret;
}

void Parser::genEndArrayAssign(array_op_params params)
{
        // Load i
        std::ostringstream ss_il;
        unsigned int temp_i_reg = this->scope->reg_ct_local++; // reg that holds value of i
        ss_il << "  %"  << temp_i_reg << " = load i64, i64* %" << params.reg_i << ", align 8";
        this->scope->writeCode(ss_il.str());


        // Increment i
        std::ostringstream ss_inc;
        unsigned int inc_i_reg = this->scope->reg_ct_local++; // reg that holds value of i++
        ss_inc << "  %"  << inc_i_reg << " = add i64 %" << temp_i_reg<< ", 1";
        this->scope->writeCode(ss_inc.str());
        
        //Store i++ in i
        std::ostringstream ss_store;
        ss_store << "  store i64 %" << inc_i_reg << ", i64* %" << params.reg_i << ", align 8";
        this->scope->writeCode(ss_store.str());

        this->genLoopEnd(params.start_label, params.end_label);
}