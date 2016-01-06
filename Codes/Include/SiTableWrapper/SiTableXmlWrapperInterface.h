#ifndef _SiTableXmlWrapperInterface_h_
#define _SiTableXmlWrapperInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/Deleter.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

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

template<typename T>
inline T  GetXmlAttrValue(xmlNodePtr node, const xmlChar *attrName)
{
    SharedXmlChar attrValue(xmlGetProp(node, attrName), XmlCharDeleter());
    char *ptr = (char *)attrValue.get();
    if (ptr == nullptr || strlen(ptr) == 0)
    {
        return 0;
    }

    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X'))
        return (T)strtol(ptr, nullptr, 16);

    return (T)strtol(ptr, nullptr, 10);
}
    
template<>
inline SharedXmlChar GetXmlAttrValue<SharedXmlChar>(xmlNodePtr node, const xmlChar *attrName)
{
    SharedXmlChar attrValue(xmlGetProp(node, attrName), XmlCharDeleter());

    return attrValue;
}   

template<>
inline std::string GetXmlAttrValue<std::string>(xmlNodePtr node, const xmlChar *attrName)
{
    std::string attrValue((char*)xmlGetProp(node, attrName));

    return attrValue;
} 

template<typename T>
inline T  GetXmlContent(xmlNodePtr node)
{
    SharedXmlChar content(xmlNodeGetContent(node), XmlCharDeleter());
    char *ptr = (char *)content.get();
    if (ptr == nullptr || strlen(ptr) == 0)
    {
        return 0;
    }

    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X'))
        return (T)strtol(ptr, nullptr, 16);

    return (T)strtol(ptr, nullptr, 10);
}
    
template<>
inline SharedXmlChar GetXmlContent<SharedXmlChar>(xmlNodePtr node)
{
    SharedXmlChar content(xmlNodeGetContent(node), XmlCharDeleter());

    return content;
}   

template<>
inline std::string GetXmlContent<std::string>(xmlNodePtr node)
{
    std::string content((char*)xmlNodeGetContent(node));

    return content;
} 

/**********************class SiTableXmlWrapperInterface**********************/
template<typename TsPacket, typename SiTable>
class SiTableXmlWrapperInterface
{
public:
    SiTableXmlWrapperInterface() {};
    virtual ~SiTableXmlWrapperInterface() {};

    virtual void Select(TsPacket &tsPacket, char *xmlFilename, TableId &tableId) = 0;
};

/**********************class NitXmlWrapper**********************/
template<typename TsPacket, typename SiTable>
class NitXmlWrapper: public SiTableXmlWrapperInterface<TsPacket, SiTable>
{
public:
    NitXmlWrapper() {};
    virtual ~NitXmlWrapper() {};

    void Select(TsPacket &tsPacket, char *xmlFilename, TableId &tableId)
    {
        cout << "Reading " << xmlFilename << endl;

        shared_ptr<xmlDoc> doc;
        for (int i = 0; i < 10 && doc == nullptr; ++i)
        {
            if (i != 0)
                SleepEx(10, true);
            doc.reset(xmlParseFile(xmlFilename), XmlDocDeleter());
        }
        assert(doc != nullptr);

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        assert(xpathCtx != nullptr);

        xmlChar *xpathExpr = (xmlChar*)"/Root/Network[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        assert(nodes != nullptr && nodes->nodeNr == 1);
        
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            NetId networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateInstance(tableId, networkId, versionNumber);

            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                if (xmlStrcmp(node->name, (xmlChar*)"Descriptors") == 0)
                {
                    AddDescriptor(*siTable, node);
                }
                else if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
                {
                    uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
                    uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

                    siTable->AddTs(tsId, onId);
            
                    xmlNodePtr child = xmlFirstElementChild(node);
                    AddTsDescriptor(*siTable, tsId, child);
                }
            }
        
            tsPacket.AddSiTable(siTable);
        }
    }

private:
    void AddDescriptor(SiTable &siTable, xmlNodePtr& node) const
    {
        assert(xmlStrcmp(node->name, (const xmlChar*)"Descriptors") == 0);
        for (xmlNodePtr cur = xmlFirstElementChild(node); 
            cur != nullptr; 
            cur = xmlNextElementSibling(cur))
        {
            uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
            //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
            std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

            siTable.AddDescriptor(data);
        }   
    }

    void AddTsDescriptor(SiTable &siTable, uint16_t tsId, xmlNodePtr& node) const
    {
        assert(xmlStrcmp(node->name, (const xmlChar*)"Descriptors") == 0);

        for (xmlNodePtr cur = xmlFirstElementChild(node); 
            cur != nullptr; 
            cur = xmlNextElementSibling(cur))
        {
            uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
            //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
            std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

            siTable.AddTsDescriptor(tsId, data);
        }
    }
};

#endif