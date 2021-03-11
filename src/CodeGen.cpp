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

        if (dest_type.is_array){
                // TODO Implement memcpy
        } else {
                if (type_holder_cmp(dest_type, expr_type)){
                        // Both are the same
                        // store <type> %expression, <type>* %destination
                        genStoreReg(dest_type.type, d-1, dest_type.reg_ct, dest_type._is_global);                      
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
        this->scope->reg_ct_local++; // Not sure why, but there is always an unused reg after parameter list
}

void Parser::genProgramBodyEnd()
{
        this->scope->writeCode("  ret i32 0");
        this->scope->writeCode("}");
}

void Parser::genProcedureHeader(symbol_t symbol, std::string name)
{
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
                ss << get_llvm_type(symbol.parameter_type_arr[i].type) << " %" << this->scope->reg_ct_local;
                this->scope->reg_ct_local++;
        }
        this->scope->reg_ct_local++; // Not sure why, but there is always an unused reg after parameter list
        
        ss << ") {";
        this->scope->writeCode(ss.str());
        
}

void Parser::genProcedureEnd()
{
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

        ss << "  %" << d << " = call " << get_llvm_type(symbol.variable_type.type) << " @" << name << symbol._procedure_ct << "(";

        auto it = regs.begin();
        for (unsigned int i = 0; i < symbol.parameter_ct; i++){
                if (i != 0){
                        ss << ", ";
                }
                ss << get_llvm_type(symbol.parameter_type_arr[i].type) << " %" << *it;
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

unsigned int Parser::genTerm(token_type_e op, token_type_e type, unsigned int reg_a, unsigned int reg_b)
{
        std::ostringstream ss;
        unsigned int d = this->scope->reg_ct_local;

        if ( op == T_OP_TERM_DIVIDE){
                if ( type == T_RW_FLOAT){
                        ss << "  %" << d << " = fdiv float %" << reg_a << ", %" << reg_b;
                } else { // T_RW_INTEGER
                        ss << "  %" << d << " = sdiv i32 %" << reg_a << ", %" << reg_b;
                }
        } else { //T_OP_TERM_MULTIPLY
                if ( type == T_RW_FLOAT){
                        ss << "  %" << d << " = fmul float %" << reg_a << ", %" << reg_b;
                } else { // T_RW_INTEGER
                        ss << "  %" << d << " = mul nsw i32 %" << reg_a << ", %" << reg_b;
                }
        }

        this->scope->writeCode(ss.str());
        this->scope->reg_ct_local++;
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
                        ss0 << "  %" << this->scope->reg_ct_local << " = trunc i32 %" << reg_a << " to i1";
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
                        ss2 << "  %" << this->scope->reg_ct_local << " = trunc i32 %" << reg_b << " to i1";
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

unsigned int Parser::genArithOp(token_type_e op, token_type_e type_a, unsigned int reg_a, token_type_e type_b, unsigned int reg_b)
{
        unsigned int d;
        

        if ( (type_a == T_RW_FLOAT) || (type_b == T_RW_FLOAT)) {
                if ( type_a == T_RW_INTEGER){
                        // convert to float
                        // %6 = sitofp i32 %5 to float
                        std::ostringstream ss0;
                        ss0 << "  %" << this->scope->reg_ct_local << " = sitofp i32 %" << reg_a << " to float";
                        reg_a = this->scope->reg_ct_local;
                        this->scope->reg_ct_local++;
                        this->scope->writeCode(ss0.str());
                } else if (type_b == T_RW_INTEGER) {
                        // convert to float
                        std::ostringstream ss0;
                        ss0 << "  %" << this->scope->reg_ct_local << " = sitofp i32 %" << reg_b << " to float";
                        reg_b = this->scope->reg_ct_local;
                        this->scope->reg_ct_local++;
                        this->scope->writeCode(ss0.str());
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