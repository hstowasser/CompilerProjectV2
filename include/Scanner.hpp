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
        void parseIdentifier(FileReader *reader, token_t *token);
        void parseDigit(FileReader *reader, token_t *token);
        void parseSymbol(FileReader *reader, token_t *token);
        void parseString(FileReader *reader, token_t *token);
        token_type_e checkReservedWord(char* word);
public:
        Scanner();
        ~Scanner();

        void scanFile(std::string filename, std::list<token_t>* token_list);

        token_t* scanToken(FileReader *reader);
};

