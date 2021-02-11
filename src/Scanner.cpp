#include "Scanner.hpp"

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

        } else if (c_class == CHR_DIGIT){
                // Parse Digit

        } else if (c_class == CHR_SYMBOL){
                // Parse Symbol

        }

        return token;
}

std::list<token> Scanner::scanFile(std::string filename)
{
        FileReader *reader;

        std::list<token> token_list;

        if (reader == NULL) {
                free(reader);
        }
        reader = new FileReader(filename);

        token_t token;
        do {
                token = this->scanToken(reader);
                token_list.push_back(token);
        } while (token.type != EOF);

        free(reader);
        return token_list;
}