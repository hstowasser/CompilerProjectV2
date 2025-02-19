#include "Scanner.hpp"
#include <string.h>

static bool error = false;

#define error_printf(reader, fmt, ...) printf("ERROR: Line %d - " fmt, (reader)->getLineNum(), ##__VA_ARGS__); error = true;

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
        const int res_word_ct = 19;
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
        reserved_list[9] = "PROCEDURE";
        reserved_list[10] = "VARIABLE";
        reserved_list[11] = "IF";
        reserved_list[12] = "THEN";
        reserved_list[13] = "ELSE";
        reserved_list[14] = "FOR";
        reserved_list[15] = "RETURN";
        reserved_list[16] = "NOT";
        reserved_list[17] = "TRUE";
        reserved_list[18] = "FALSE";

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
                // Add buffer to symbol table.
                token->setValue(std::string(buffer, i+1));

        }

        if ( i == MAX_IDENTIFIER_LENGTH){
                error_printf(reader, "Max identifier length exceeded\n");
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
                error_printf(reader, "Max digit length exceeded\n");
        }

        if (is_float){
                float temp = strtof(buffer, &endp);
                token->type = T_CONST_FLOAT;
                token->setValue(temp);
        } else {
                int temp = strtol(buffer, &endp, 10);
                token->type = T_CONST_INTEGER;
                token->setValue(temp);
        }
}

void Scanner::parseSymbol(FileReader *reader, token_t *token)
{
        char c = reader->peekc();
        char next_c = reader->peek2();

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
                        error_printf(reader, "Expected = after !\n");
                        token->type = T_UNKNOWN;
                }
                break;
        case '=':
                if (next_c == '='){
                        is_two_char_operator = true;
                        token->type = T_OP_REL_EQUAL;
                } else {
                        // ERROR
                        error_printf(reader, "Expected = after =\n");
                        token->type = T_UNKNOWN;
                }
                break;
        case '&':
                token->type = T_OP_BITW_AND;
                break;
        case '|':
                token->type = T_OP_BITW_OR;
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
                error_printf(reader, "INVALID Symbol\n");
                token->type = T_UNKNOWN;
        }

        reader->getc();
        if ( is_two_char_operator){
                reader->getc();
        }
}

void Scanner::parseString(FileReader *reader, token_t *token)
{
        char c = ' '; // Initialize to not / or "
        std::string str;

        token->type = T_CONST_STRING;

        reader->getc(); // This should be the first quote, so we'll skip it
        c = reader->getc();
        while( c != '"' && c != EOF){
                if (c == '\\'){
                        c = reader->getc(); // Skip it
                        // Don't add escape \ to string
                }
                str.append(1, c);
                c = reader->getc();
        }

        
        if ( c == EOF ){
                error_printf(reader, "Unexpected EOF while parsing string\n");
        } else {
                // Add string to symbol table
                token->setValue(str);
        }

        return;
}

token_t* Scanner::scanToken(FileReader *reader)
{
        char c;
        char_class_t c_class;
        token_t* token = new token_t();

        this->skipCommentsAndWhiteSpace(reader);

        token->line_num = reader->getLineNum();

        c = reader->peekc();
        c_class = this->getCharClass(c);

        if ( c == EOF ){
                // Return EOF token
                token->type = T_EOF;
        } else if(c_class == CHR_LETTER){
                // Parse Identifier
                this->parseIdentifier(reader, token);
        } else if (c_class == CHR_DIGIT){
                // Parse Digit
                this->parseDigit(reader, token);
        } else if (c_class == CHR_SYMBOL){
                // Parse Symbol
                this->parseSymbol(reader, token);
        }

        return token;
}

bool Scanner::scanFile(std::string filename, std::list<token_t>* token_list)
{
        FileReader *reader;
        token_t* token;

        error = false;

        reader = new FileReader(filename);


        do {
                token = this->scanToken(reader);
                token_list->push_back(*token);
        } while (token->type != T_EOF);

        delete reader;
        return error;
}