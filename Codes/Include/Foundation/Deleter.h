#ifndef _Deleter_h_
#define _Deleter_h_

/* Foundation */
#include "Include/Foundation/Type.h"

/******************class ArrayDeleter******************/
/*
ArrayDeleter, auxiliary class used by shared_ptr<char>.  
Example:
{
    shared_ptr<char> buffer(new char[128], ArrayDeleter<char>());
}
*/
template<typename T>
class ArrayDeleter: public std::unary_function<T, void>
{
public:
    ArrayDeleter()
    {}

    result_type operator()(argument_type *ptr) const
    {
        delete[] ptr;
    }
};

/******************class ScalarDeleter******************
Example:
{
    for_each(xxx.begin(), xxx.end(), ScalarDeleter<Xxx>());
}
*/
template<typename T>
class ScalarDeleter: public std::unary_function<T, void>
{
public:
    ScalarDeleter()
    {}

    result_type operator()(argument_type *ptr) const
    {
        delete ptr;
    }
};

#endif