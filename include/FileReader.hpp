#pragma once

#include <string>
#include <iostream>
#include <fstream>

class FileReader
{
private:
    std::ifstream *fin_;
    unsigned int linenum_;
public:
    FileReader(std::string filename);
    ~FileReader();

    char peekc(); // Get next char without moving pointer
    char getc(); // Get next char
    unsigned int getLineNum();
    bool eof();
};
