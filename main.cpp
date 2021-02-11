#include "include/Scanner.hpp"
#include "include/Token.hpp"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
        Scanner *scanner = new Scanner();
        
        list<token_t> token_list = scanner->scanFile("test.txt");
        
        list<token_t>::iterator itr;
        for ( itr = token_list.begin(); itr != token_list.end(); itr++){
                print_token(*itr);
        }

        free(scanner);

        return 0;
        }