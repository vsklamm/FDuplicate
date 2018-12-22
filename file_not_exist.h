#ifndef FILE_NOT_EXIST_H
#define FILE_NOT_EXIST_H

#include <exception>

class file_not_exist: public std::exception
{
    file_not_exist(int line_, const char* func_);

    virtual const char* what() const throw();

    int get_line() const;
    const char* get_func() const;

private:

    int line;
    const char* func;
};

#endif // FILE_NOT_EXIST_H
