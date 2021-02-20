#include "include/Scanner.hpp"
#include "include/Token.hpp"
#include "include/Parser.hpp"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
        Scanner *scanner = new Scanner();
        Parser *parser = new Parser();

        list<token_t> token_list = scanner->scanFile("test.txt");
        
        list<token_t>::iterator itr;
        for ( itr = token_list.begin(); itr != token_list.end(); itr++){
                //print_token(*itr);
        }

        free(scanner);

        parser->parse(token_list);

        return 0;
        }