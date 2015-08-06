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

/**********************class NitXmlWrapper**********************/
template<typename Nit>
class NitXmlWrapper: public NitWrapper<Nit>
{
public:
    typedef NitWrapper<Nit> MyBase;
    typedef NitXmlWrapper<Nit> MyType;    

    NitXmlWrapper(const std::string thXmlFileName, Trigger& theTrigger)
        : MyBase(theTrigger), xmlFileName(thXmlFileName)
    {
    }

    template<typename RetType>
    RetType  GetXmlAttrValue(xmlNodePtr node, const xmlChar *attrName) const;
  
    template<>
    uchar_t GetXmlAttrValue<uchar_t>(xmlNodePtr node, const xmlChar *attrName) const
    {
        std::shared_ptr<xmlChar> attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        char *ptr = (char *)attrValue.get();
        return (uchar_t)strtol(ptr, nullptr, 16);
    }

    template<>
    uint16_t GetXmlAttrValue<uint16_t>(xmlNodePtr node, const xmlChar *attrName) const
    {
        std::shared_ptr<xmlChar> attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        char *ptr = (char *)attrValue.get();
        return (uint16_t)strtol(ptr, nullptr, 10);
    }

    template<>
    std::shared_ptr<xmlChar> GetXmlAttrValue<std::shared_ptr<xmlChar>>(xmlNodePtr node, const xmlChar *attrName) const
    {
        std::shared_ptr<xmlChar> attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        return attrValue;
    }

    template<typename Owner>
    void AddDescriptor(Owner& owner, xmlNodePtr& node, xmlChar* child) const
    {
        if (xmlStrcmp(node->name, child) == 0)
        {
            for (xmlNodePtr cur = xmlFirstElementChild(node); 
                cur != nullptr; 
                cur = xmlNextElementSibling(cur))
            {
                uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
                std::shared_ptr<xmlChar> data = GetXmlAttrValue<std::shared_ptr<xmlChar>>(cur, (const xmlChar*)"Data");
                owner.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
            }
        }
    }

    std::error_code FillNit(Nit& nit) const;

private:
    std::string xmlFileName;
};

#endif