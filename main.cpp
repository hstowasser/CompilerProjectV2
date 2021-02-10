#include "include/Scanner.hpp"
#include "include/Token.hpp"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream fin;
    fin.open("test.txt", ios::in);

    char c;
    int num_lines=0;

    while (!fin.eof()){
        fin.get(c);
        cout << c;
        if ( c == '\n'){
            num_lines++;
        }
    }

    return 0;
}