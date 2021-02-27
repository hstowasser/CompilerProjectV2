#include "FileReader.hpp"
#include <iostream>
#include <fstream>

using namespace std;

FileReader::FileReader(std::string filename)
{
        fin_ = new std::ifstream();
        fin_->open(filename, std::ios::in);
        linenum_ = 1;
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
        if (fin_->eof()){
                return EOF;
        } else {
                return fin_->peek();
        }
}

char FileReader::peek2()
{
        char c;
        if (fin_->eof()){
                c = EOF;
        }else{
                fin_->get(c);
                c = this->peekc();
                fin_->seekg(-1, ios::cur);
        }
        return c;
}

char FileReader::getc()
{
        char c;
        if (fin_->eof()){
                return EOF;
        }else{
                fin_->get(c);
                if (c == '\n'){
                        linenum_++;
                }
                return c;
        }
}

bool FileReader::eof()
{
        return fin_->eof();
}