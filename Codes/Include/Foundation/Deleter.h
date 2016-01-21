#ifndef _Deleter_h_
#define _Deleter_h_

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

/* Foundation */
#include "Include/Foundation/Type.h"

/******************class ArrayDeleter******************/
/*
ArrayDeleter, auxiliary class used by shared_ptr<char>.  
Example:
{
    shared_ptr<char> buffer(new char[128], ArrayDeleter());
}
*/
class ArrayDeleter
{
public:
    ArrayDeleter()
    {}

    template<typename T>
    void operator()(const T *ptr) const
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
class ScalarDeleter
{
public:
    ScalarDeleter()
    {}

    template<typename T>
    void operator()(const T *ptr) const
    {
        delete ptr;
    }
};

/*
XmlDocDeleter, auxiliary class used by shared_ptr<xmlDoc>.  Example:
{
    const char* xmlFile = "../XmlFiles/AddressListXml.xml";
    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFile), XmlDocDeleter());
}
*/
class XmlDocDeleter
{
public:
    XmlDocDeleter()
    {}

    void operator()(xmlDoc* ptr) const
    {
        xmlFreeDoc(ptr);
    }
};

class XmlCharDeleter
{
public:
    XmlCharDeleter()
    {}

    void operator()(xmlChar *ptr) const
    {
        xmlFree(ptr);
    }
};

class xmlXPathContextDeleter
{
public:
    xmlXPathContextDeleter()
    {}

    void operator()(xmlXPathContext *ptr) const
    {
        xmlXPathFreeContext(ptr);
    }
};

class xmlXPathObjectDeleter
{
public:
    xmlXPathObjectDeleter()
    {}

    void operator()(xmlXPathObject *ptr) const
    {
        xmlXPathFreeObject(ptr);
    }
};

#endif