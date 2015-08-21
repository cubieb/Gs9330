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

typedef std::shared_ptr<xmlChar> SharedXmlChar;

/**********************class NitXmlWrapper**********************/
template<typename Nit>
class NitXmlWrapper: public NitWrapper<Nit>
{
public:
    typedef NitWrapper<Nit> MyBase;
    typedef NitXmlWrapper<Nit> MyType;    

    NitXmlWrapper(Trigger& theTrigger, const std::string thXmlFileName)
        : MyBase(theTrigger), xmlFileName(thXmlFileName)
    {
    }

    template<typename T>
    T  GetXmlAttrValue(xmlNodePtr node, const xmlChar *attrName) const
    {
        SharedXmlChar attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        char *ptr = (char *)attrValue.get();
        return (T)strtol(ptr, nullptr, 16);
    }
    
    template<>
    SharedXmlChar GetXmlAttrValue<SharedXmlChar>(xmlNodePtr node, const xmlChar *attrName) const
    {
        SharedXmlChar attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        return attrValue;
    }

    void Start() const;

    void AddDescriptor(Nit& nit, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor(Nit& nit, uint16_t onId, xmlNodePtr& node, xmlChar* child) const;
    std::error_code FillNit(Nit& nit) const;    
    
private:
    std::string xmlFileName;
};

#endif