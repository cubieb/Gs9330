#ifndef _XmlDataWrapper_h_
#define _XmlDataWrapper_h_
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "DataWrapper.h"
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

template<typename Nit, typename DescriporFactory>
class NitXmlWrapper: public NitWrapper<Nit, DescriporFactory>
{
public:
    typedef NitWrapper<Nit, DescriporFactory> MyBase;
    typedef NitXmlWrapper<Nit, DescriporFactory> MyType;    

    NitXmlWrapper(const std::string thXmlFileName, DescriporFactory& theFactory, Trigger& theTrigger)
        : MyBase(theFactory, theTrigger), xmlFileName(thXmlFileName)
    {
    }

    std::error_code FillNit(Nit& nit) const
    {
        error_code err;
        return err;
    }

private:
    std::string xmlFileName;
};

#endif