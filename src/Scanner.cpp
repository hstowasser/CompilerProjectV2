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

void Scanner::skipWhiteSpace(FileReader *reader)
{
        char_class_t c_class;
        char c;
        do {
                c = reader->peekc();
                c_class == this->getCharClass(c);
                if (c_class == CHR_WHITE_SPACE) {
                        reader->getc();
                }
        } while (c_class == CHR_WHITE_SPACE && c != EOF);
}

void Scanner::skipCommentsAndWhiteSpace(FileReader *reader)
{
        char_class_t c_class = CHR_UNKNOWN;
        char c = 'x'; // Initialize to not EOF

        do {
                // Skip Whitespace
                c = getc(fp);
                c_class = get_char_class(c);
                if (c == EOF) {
                        error = SCAN_EOF;
                } else if (c_class == CHR_WHITE_SPACE) {
                        error = skip_whitespace(fp);
                } else if (check_for_comment(fp, c)) {
                        error = skip_comments(fp);
                } else {
                        break;
                }
        } while (error == SCAN_OK);

        // Roll back pointer to first char after comments/whitespaces
        if (fseek(fp, -1, SEEK_CUR) != 0) {
                printf("Error scanning \n");
                error = SCAN_ERROR;
        }
        return error;
}

token_t Scanner::scanToken(FileReader *reader)
{
        this->skipCommentsAndWhiteSpace(reader);
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