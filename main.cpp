#include "include/Scanner.hpp"
#include "include/Token.hpp"
#include "include/Parser.hpp"
#include "include/Scope.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <cstring>

#include <unistd.h>

using namespace std;

void destroy_list(list<token_t>* token_list)
{
        list<token_t>::iterator itr;
        for ( itr = token_list->begin(); itr != token_list->end(); itr++){
                itr->destroy();
        }  
}

int main(int argc, char **argv)
{
        if ( argc != 2) {
                printf("Invalid input argument count.\n");
                printf("Correct usage is :\n");
                printf("   geecc <file>\n");
                return -1;
        }

        std::ostringstream ss;
        
        ss << argv[1];
        std::string filename = ss.str();

        Scope *scope = new Scope();
        Scanner *scanner = new Scanner();
        Parser *parser = new Parser(scope);
        list<token_t> token_list;

        bool error = scanner->scanFile(argv[1], &token_list);
        delete scanner;
        if( error ){
                printf("Fail\n");
                return -1;
        }

        // list<token_t>::iterator itr;
        // for ( itr = token_list.begin(); itr != token_list.end(); itr++){
        //         print_token(*itr);
        // }

        bool ret = parser->parse(token_list);
        if (ret){
                printf("Pass\n");
        }else{
                printf("Fail\n");
                return -1;
        }

        // Write llvm to temp file
        std::ostringstream ss_ll;
        ss_ll << filename << ".ll";
        std::string asm_file = ss_ll.str();
        scope->PrintCodeToFile(asm_file);

        std::ostringstream ss_cmd;
        ss_cmd << "llvm-as " << asm_file << " -o " << parser->program_name;
        system(ss_cmd.str().c_str());

        //scope->PrintScope();
        //scope->PrintCode();

        destroy_list(&token_list);

        return 0;
}
