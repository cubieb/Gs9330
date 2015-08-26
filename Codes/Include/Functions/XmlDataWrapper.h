#ifndef _XmlDataWrapper_h_
#define _XmlDataWrapper_h_
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "SystemError.h"

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

/**********************class XmlDataWrapper**********************/
template<typename Table>
class XmlDataWrapper: public DataWrapper<Table>
{
public:
    typedef DataWrapper<Table> MyBase;
    typedef XmlDataWrapper<Table> MyType;

    XmlDataWrapper(Trigger& trigger, const std::string thXmlFileName)
        : MyBase(trigger), xmlFileName(thXmlFileName)
    {}

    virtual ~XmlDataWrapper() 
    {}    

    void Start() const
    {
        trigger(*this);
    }

    template<typename T>
    T  GetXmlAttrValue(xmlNodePtr node, const xmlChar *attrName) const
    {
        SharedXmlChar attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        char *ptr = (char *)attrValue.get();
        if (ptr[0] == '0' && ptr[1] == 'x')
            return (T)strtol(ptr, nullptr, 16);

        return (T)strtol(ptr, nullptr, 10);
    }
    
    template<>
    SharedXmlChar GetXmlAttrValue<SharedXmlChar>(xmlNodePtr node, const xmlChar *attrName) const
    {
        SharedXmlChar attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        return attrValue;
    }

    virtual std::error_code Fill(Table&) const = 0;

protected:
    std::string xmlFileName;
};

/**********************class NitXmlWrapper**********************/
template<typename Table>
class NitXmlWrapper: public XmlDataWrapper<Table>
{
public:
    typedef XmlDataWrapper<Table> MyBase;
    typedef NitXmlWrapper<Table> MyType;    

    NitXmlWrapper(Trigger& trigger, const std::string xmlFileName)
        : MyBase(trigger, xmlFileName)
    {
    }

    void AddDescriptor(Table& nit, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor(Table& nit, uint16_t onId, xmlNodePtr& node, xmlChar* child) const;
    std::error_code Fill(Table& nit) const;
};

/**********************class SdtXmlWrapper**********************/
template<typename Table>
class SdtXmlWrapper: public XmlDataWrapper<Table>
{
public:
    typedef XmlDataWrapper<Table> MyBase;
    typedef SdtXmlWrapper<Table> MyType;    

    SdtXmlWrapper(Trigger& trigger, const std::string xmlFileName)
        : MyBase(trigger, xmlFileName)
    {
    }

    void AddService(Table& sdt, xmlNodePtr& node, xmlChar* child) const;
    std::error_code Fill(Table& sdt) const;
};

/**********************class BatXmlWrapper**********************/
template<typename Table>
class BatXmlWrapper: public XmlDataWrapper<Table>
{
public:
    typedef XmlDataWrapper<Table> MyBase;
    typedef BatXmlWrapper<Table> MyType;    

    BatXmlWrapper(Trigger& trigger, const std::string xmlFileName)
        : MyBase(trigger, xmlFileName)
    {
    }

    void AddDescriptor(Table& nit, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor(Table& nit, uint16_t onId, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor0x41(Table& bat, uint16_t tsId,
                             xmlNodePtr& node, xmlChar* child) const;

    std::error_code Fill(Table& nit) const;
};

#endif