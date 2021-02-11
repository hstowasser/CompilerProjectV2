#include "FileReader.hpp"
#include <iostream>
#include <fstream>

using namespace std;

FileReader::FileReader(std::string filename)
{
        fin_ = new std::ifstream();
        fin_->open(filename, std::ios::in);
}

FileReader::~FileReader()
{
        fin_->close();
        free(fin_);
}

unsigned int FileReader::getLineNum() 
{
        return linenum_;
}

char FileReader::peekc()
{
        char c;
        c = fin_->peek();
        return c;
}

char FileReader::getc()
{
        char c;
        fin_->get(c);
        if (c == '\n'){
                linenum_++;
        }
        return c;
}

bool FileReader::eof()
{
        return fin_->eof();
}