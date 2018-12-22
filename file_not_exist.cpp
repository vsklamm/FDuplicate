#include "file_not_exist.h"

file_not_exist::file_not_exist(int line_, const char *func_)
    : line (line_), func (func_)
{
}

const char *file_not_exist::what() const throw()
{
    return "Trying to open a nonexistent file";
}

int file_not_exist::get_line() const { return line; }

const char *file_not_exist::get_func() const { return func; }
