#include "include/Scanner.hpp"
#include "include/Token.hpp"
#include "include/FileReader.hpp"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
        FileReader *reader = new FileReader("test.txt");
        

        char c;
        int num_lines=0;

        while (!reader->eof()){
                c = reader->getc();
                if ( c == '\n'){
                cout << " " << reader->getLineNum() << " ";
                }
                cout << c;
        }

        free(reader);

        return 0;
        }