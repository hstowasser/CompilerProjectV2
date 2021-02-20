#include "Scanner.hpp"
#include <string.h>

Scanner::Scanner()
{
}

Scanner::~Scanner()
{
}

char_class_t Scanner::getCharClass(char c)
{
        char_class_t cclass = CHR_UNKNOWN;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                cclass = CHR_LETTER;
        } else if (c >= '0' && c <= '9') {
                cclass = CHR_DIGIT;
        } else if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                cclass = CHR_WHITE_SPACE;
        } else {
                cclass = CHR_SYMBOL;
        }
        return cclass;
}

// void Scanner::skipWhiteSpace(FileReader *reader)
// {
//         char_class_t c_class;
//         char c;
//         do {
//                 c = reader->peekc();
//                 c_class == this->getCharClass(c);
//                 if (c_class == CHR_WHITE_SPACE) {
//                         reader->getc();
//                 }
//         } while (c_class == CHR_WHITE_SPACE && c != EOF);
// }

void Scanner::skipLineComment(FileReader *reader)
{
        char c;
        do{
                c = reader->getc();
        } while ( c != '\n' && c != EOF);
}

void Scanner::skipBlockComment(FileReader *reader)
{
        char c = 'x';
        char last_c = 'x';
        while ( !(last_c == '*' && c == '/') ){ // Stop when we see "*/"
                if ( c == EOF ){
                        // END of file reached
                        break;
                } else if (last_c == '/' && c == '*') {
                        // If we find the start of a new block comment
                        // recursively call the skip function;
                        this->skipBlockComment(reader);
                        c = reader->getc();
                        last_c = 'x'; // Reset because we skipped forward
                }else{
                        last_c = c;
                        c = reader->getc();
                }
        }
}

void Scanner::skipCommentsAndWhiteSpace(FileReader *reader)
{
        char_class_t c_class = CHR_UNKNOWN;
        char c1;
        char c2;
        do {
                // Skip Whitespace
                c1 = reader->peekc();
                c2 = reader->peek2();
                c_class = this->getCharClass(c1);
                if (c_class == CHR_WHITE_SPACE) {
                        // Skip white space
                        reader->getc(); // Increment
                } else if (c1 == '/' && c2 == '/') {
                        // Skip Line Comment
                        this->skipLineComment(reader);
                } else if (c1 == '/' && c2 == '*') {
                        // Skip Block Comment
                        reader->getc(); // Remove start of block comment
                        reader->getc();
                        this->skipBlockComment(reader);
                } else {
                        break;
                }
        } while (1);
}

token_type_e Scanner::checkReservedWord(char* word)
{
        token_type_e ret = T_IDENTIFIER; // We set it to identifier by default
        const int res_word_ct = 21;
        const char *reserved_list[res_word_ct];
        reserved_list[0] = "PROGRAM"; // NOTE: Order must match that in token_type_e
        reserved_list[1] = "IS";
        reserved_list[2] = "BEGIN";
        reserved_list[3] = "END";
        reserved_list[4] = "GLOBAL";
        reserved_list[5] = "INTEGER";
        reserved_list[6] = "FLOAT";
        reserved_list[7] = "STRING";
        reserved_list[8] = "BOOL";
        reserved_list[9] = "ENUM";
        reserved_list[10] = "PROCEDURE";
        reserved_list[11] = "VARIABLE";
        reserved_list[12] = "IF";
        reserved_list[13] = "THEN";
        reserved_list[14] = "ELSE";
        reserved_list[15] = "FOR";
        reserved_list[16] = "RETURN";
        reserved_list[17] = "NOT";
        reserved_list[18] = "TYPE";
        reserved_list[19] = "TRUE";
        reserved_list[20] = "FALSE";

        for (int index = 0; index < res_word_ct; index++){
                if ( strcmp(word, reserved_list[index]) == 0 ){
                        ret = (token_type_e)(T_RW_PROGRAM + index);
                        break;
                }
        }
        return ret;
}

void Scanner::parseIdentifier(FileReader *reader, token_t *token)
{
        size_t i = 0;
        char c;
        char_class_t c_class;
        char buffer[MAX_IDENTIFIER_LENGTH];

        c = reader->peekc();
        c_class = this->getCharClass(c);
        while ( c != EOF && (c_class == CHR_LETTER || c_class == CHR_DIGIT) && i < MAX_IDENTIFIER_LENGTH){
                reader->getc(); // Increment
                buffer[i] = toupper(c);
                c = reader->peekc();
                c_class = this->getCharClass(c);
                i++;
        }
        buffer[i] = '\0';

        // Check for reserved word
        token->type = this->checkReservedWord(buffer);

        if (token->type == T_IDENTIFIER){
                // TODO Add buffer to symbol table.

        }

        if ( i == MAX_IDENTIFIER_LENGTH){
                // TODO Handle error, max identifier length exceeded
        }
        return;
}

void Scanner::parseDigit(FileReader *reader, token_t *token)
{
        char buffer[MAX_DIGIT_LENGTH];
        char* endp;
        int i = 0;
        char c = reader->peekc();
        char_class_t c_class = this->getCharClass(c);

        bool is_float = false;
        while ( c != EOF && (c_class == CHR_DIGIT || c == '.' ) && i < MAX_IDENTIFIER_LENGTH){
                reader->getc(); // Increment
                buffer[i] = c;
                if ( c == '.'){
                        is_float = true;
                }
                c = reader->peekc();
                c_class = this->getCharClass(c);
                i++;
        }

        if ( i >= MAX_DIGIT_LENGTH){
                // TODO Handle error, max digit length exceeded
        }

        if (is_float){
                float temp = strtof(buffer, &endp);
                token->type = T_CONST_FLOAT;
                // TODO Add float to symbol tree?
                // token->value = tree_add( &scanner_inst->symbol_tree, &temp, sizeof(float));
        } else {
                int temp = strtol(buffer, &endp, 10);
                token->type = T_CONST_INTEGER;
                // TODO Add int to symbol tree?
                // token->value = tree_add( &scanner_inst->symbol_tree, &temp, sizeof(int));
        }
}

void Scanner::parseSymbol(FileReader *reader, token_t *token)
{
        char c = reader->peekc();
        char next_c = reader->peekc();

        bool is_two_char_operator = false;

        switch (c){
        case '"':
                // Special case: We don't want to rewind fp after parse_string()
                this->parseString(reader, token);
                return;
        case '(':
                token->type = T_SYM_LPAREN;
                break;
        case ')':
                token->type = T_SYM_RPAREN;
                break;
        case '[':
                token->type = T_SYM_LBRACKET;
                break;
        case ']':
                token->type = T_SYM_RBRACKET;
                break;
        case '{':
                token->type = T_SYM_LBRACE;
                break;
        case '}':
                token->type = T_SYM_RBRACE;
                break;
        case ';':
                token->type = T_SYM_SEMICOLON;
                break;
        case ':':
                if (next_c == '='){
                        is_two_char_operator = true;
                        token->type = T_OP_ASIGN_EQUALS;
                } else {
                        token->type = T_SYM_COLON;
                }
                break;
        case '.':
                token->type = T_SYM_PERIOD;
                break;
        case ',':
                token->type = T_SYM_COMMA;
                break;
        case '>':
                if (next_c == '='){
                        is_two_char_operator = true;
                        token->type = T_OP_REL_GREATER_EQUAL;
                } else {
                        token->type = T_OP_REL_GREATER;
                }
                break;
        case '<':
                if (next_c == '='){
                        is_two_char_operator = true;
                        token->type = T_OP_REL_LESS_EQUAL;
                } else {
                        token->type = T_OP_REL_LESS;
                }
                break;
        case '!':
                if (next_c == '='){
                        is_two_char_operator = true;
                        token->type = T_OP_REL_NOT_EQUAL;
                } else {
                        // ERROR
                        // error = SCAN_ERROR;
                        token->type = T_UNKNOWN;
                }
                break;
        case '=':
                if (next_c == '='){
                        is_two_char_operator = true;
                        token->type = T_OP_REL_EQUAL;
                } else {
                        // ERROR
                        // error = SCAN_ERROR;
                        token->type = T_UNKNOWN;
                }
                break;
        case '&':
                token->type = T_OP_LOGI_AND;
                break;
        case '|':
                token->type = T_OP_LOGI_OR;
                break;
        case '*':
                token->type = T_OP_TERM_MULTIPLY;
                break;
        case '/':
                token->type = T_OP_TERM_DIVIDE;
                break;
        case '+':
                token->type = T_OP_ARITH_PLUS;
                break;
        case '-':
                token->type = T_OP_ARITH_MINUS;
                break;
        default:
                // Unknown Symbol
                // error = SCAN_ERROR;
                token->type = T_UNKNOWN;
        }

        reader->getc();
        if ( is_two_char_operator){
                reader->getc();
        }
}

void Scanner::parseString(FileReader *reader, token_t *token)
{
        char c;
        bool escape = false; // This holds whether or not the last char was a backslash
        int string_len = 1;
        std::string str;

        token->type = T_CONST_STRING;

        reader->getc(); // This should be the first quote, so we'll skip it
        c = reader->getc();
        escape = c == '\\';
        while ( c != '"' && escape == false && c != EOF){
                // Loop through till we find an unescaped quotation mark
                c = reader->getc();
                str.append(1, c);
                escape = c == '\\';
                string_len++;
        }

        
        if ( c == EOF ){
                // TODO Handle: Unexpected EOF while parsing string
        } else {
                // TODO Add string to symbol table
                // char* buffer = malloc(string_len);
                // fgets(buffer, string_len, fp);
                // token->value = tree_add( &scanner_inst->symbol_tree, buffer, string_len);
                // free(buffer);
                // c = getc(fp);
        }

        return;
}

token_t Scanner::scanToken(FileReader *reader)
{
        char c;
        char_class_t c_class;
        token_t token;

        this->skipCommentsAndWhiteSpace(reader);

        c = reader->peekc();
        c_class = this->getCharClass(c);

        if ( c == EOF ){
                // Return EOF token
                token.type = T_EOF;
        } else if(c_class == CHR_LETTER){
                // Parse Identifier
                this->parseIdentifier(reader, &token);
        } else if (c_class == CHR_DIGIT){
                // Parse Digit
                this->parseDigit(reader, &token);
        } else if (c_class == CHR_SYMBOL){
                // Parse Symbol
                this->parseSymbol(reader, &token);
        }

        return token;
}

std::list<token_t> Scanner::scanFile(std::string filename)
{
        FileReader *reader;
        token_t token;
        std::list<token_t> token_list;

        if (reader != NULL) {
                free(reader);
        }
        reader = new FileReader(filename);


        do {
                token = this->scanToken(reader);
                token_list.push_back(token);
        } while (token.type != T_EOF);

        free(reader);
        return token_list;
}