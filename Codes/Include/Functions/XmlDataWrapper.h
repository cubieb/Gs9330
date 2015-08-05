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
        shared_ptr<xmlChar> attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        char *ptr = (char *)attrValue.get();
        return (uchar_t)strtol(ptr, nullptr, 16);
    }

    template<>
    uint16_t GetXmlAttrValue<uint16_t>(xmlNodePtr node, const xmlChar *attrName) const
    {
        shared_ptr<xmlChar> attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        char *ptr = (char *)attrValue.get();
        return (uint16_t)strtol(ptr, nullptr, 10);
    }

    template<>
    std::shared_ptr<xmlChar> GetXmlAttrValue<std::shared_ptr<xmlChar>>(xmlNodePtr node, const xmlChar *attrName) const
    {
        std::shared_ptr<xmlChar> attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
        return attrValue;
    }

    std::error_code FillNit(Nit& nit) const
    {
        error_code err;

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlFileName.c_str()), XmlDocDeleter());
        if (doc == nullptr)
        {
            err == system_error_t::file_not_exists;
            return err;
        }
        
        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        nit.SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));
        
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        assert(xpathCtx != nullptr);

        xmlChar *xpathExpr = (xmlChar*)"/Root/Network[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        assert(nodes != nullptr);

        size_t i, size = nodes->nodeNr;
        for (i = 0; i < size; ++i)
        {
            if(nodes->nodeTab[i]->type != XML_ELEMENT_NODE)
                continue;

            node = nodes->nodeTab[i];
            nit.SetNetworkId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID"));
            nit.SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));
            std::shared_ptr<xmlChar> name = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
            nit.AddDescriptor(NetworkNameDescriptor::Tag, name.get(), strlen((const char*)name.get()) + 1);

            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                if (xmlStrcmp(node->name, (xmlChar*)"Descriptors") == 0)
                {
                    for (xmlNodePtr child = xmlFirstElementChild(node); 
                         child != nullptr; 
                         child = xmlNextElementSibling(child))
                    {
                        uchar_t tag = GetXmlAttrValue<uchar_t>(child, (const xmlChar*)"Tag");
                        std::shared_ptr<xmlChar> data = GetXmlAttrValue<shared_ptr<xmlChar>>(child, (const xmlChar*)"Data");
                        nit.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
                    }
                }

                if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
                {
                    uint16_t tsId, onId;
                    tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
                    onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

                    Nit::TransportStream& transportStream = nit.AddTransportStream(tsId, onId);
                    for (xmlNodePtr child = xmlFirstElementChild(xmlFirstElementChild(node)); 
                         child != nullptr; 
                         child = xmlNextElementSibling(child))
                    {
                        uchar_t tag = GetXmlAttrValue<uchar_t>(child, (const xmlChar*)"Tag");
                        std::shared_ptr<xmlChar> data = GetXmlAttrValue<shared_ptr<xmlChar>>(child, (const xmlChar*)"Data");
                        transportStream.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
                    }
                }
            }
        }

        xmlCleanupParser();
        return err;
    }

private:
    std::string xmlFileName;
};

#endif