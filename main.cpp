#include "include/Scanner.hpp"
#include "include/Token.hpp"
#include "include/Parser.hpp"
#include "include/Scope.hpp"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;

void destroy_list(list<token_t>* token_list)
{
        list<token_t>::iterator itr;
        for ( itr = token_list->begin(); itr != token_list->end(); itr++){
                itr->destroy();
        }  
}

int main()
{
        Scope *scope = new Scope();
        Scanner *scanner = new Scanner();
        Parser *parser = new Parser(scope);
        list<token_t> token_list;

        scanner->scanFile("test1.txt", &token_list);
        delete scanner;

        // list<token_t>::iterator itr;
        // for ( itr = token_list.begin(); itr != token_list.end(); itr++){
        //         print_token(*itr);
        // }

        parser->parse(token_list);

        scope->PrintScope();
        scope->PrintCode();

        destroy_list(&token_list);

        return 0;
}
