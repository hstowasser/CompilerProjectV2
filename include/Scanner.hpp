#pragma once
#include <string>
#include <list>
#include "FileReader.hpp"
#include "Token.hpp"

typedef enum{
        CHR_LETTER, // a-z A-Z and _
        CHR_DIGIT,
        CHR_WHITE_SPACE,
        CHR_SYMBOL,
        CHR_UNKNOWN
} char_class_t;

class Scanner
{
private:
        void skipCommentsAndWhiteSpace(FileReader *reader);
        void skipLineComment(FileReader *reader);
        void skipBlockComment(FileReader *reader);
        // void skipWhiteSpace(FileReader *reader);
        char_class_t getCharClass(char c);
public:
        Scanner();
        ~Scanner();

        std::list<token> scanFile(std::string filename);

        token_t scanToken(FileReader *reader);
};

