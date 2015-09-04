#ifndef _XmlDataWrapper_h_
#define _XmlDataWrapper_h_
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "SystemError.h"

#include "DirMonitor.h"
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

/**********************class XmlDataWrapper**********************/
template<typename Section>
class XmlDataWrapper: public DataWrapper
{
public:
    typedef DataWrapper MyBase;
    typedef XmlDataWrapper<Section> MyType;

    XmlDataWrapper(DbInsertHandler& handler, const char *xmlFileDir, const char *xmlFileMatch);
    virtual ~XmlDataWrapper();

    void Start();
    void FileIoCompletionRoutine(const char *file);

    virtual std::shared_ptr<Section> CreateSection(const char* xmlPath) const = 0;

protected:
    std::string xmlFileDir;
    std::string xmlFileRegularExp;
    DirMonitor dirMonitor;
};

/**********************class NitXmlWrapper**********************/
template<typename Section>
class NitXmlWrapper: public XmlDataWrapper<Section>
{
public:
    typedef XmlDataWrapper<Section> MyBase;
    typedef NitXmlWrapper<Section> MyType;    

    NitXmlWrapper(DbInsertHandler& handler, const char *xmlFileDir)
        : MyBase(handler, xmlFileDir, ".*nit.*\\.xml")
    {
    }

    std::shared_ptr<Section> CreateSection(const char* xmlPath) const;

private:
    void AddDescriptor(Section& nit, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor(Section& nit, uint16_t onId, xmlNodePtr& node, xmlChar* child) const;    
};

/**********************class SdtXmlWrapper**********************/
template<typename Section>
class SdtXmlWrapper: public XmlDataWrapper<Section>
{
public:
    typedef XmlDataWrapper<Section> MyBase;
    typedef SdtXmlWrapper<Section> MyType;    

    SdtXmlWrapper(DbInsertHandler& handler, const char *xmlFileDir)
        : MyBase(handler, xmlFileDir, ".*sdt.*\\.xml")
    {
    }

    std::shared_ptr<Section> CreateSection(const char* xmlPath) const;

private:
    void AddService(Section& sdt, xmlNodePtr& node, xmlChar* child) const;
};

/**********************class BatXmlWrapper**********************/
template<typename Section>
class BatXmlWrapper: public XmlDataWrapper<Section>
{
public:
    typedef XmlDataWrapper<Section> MyBase;
    typedef BatXmlWrapper<Section> MyType;    

    BatXmlWrapper(DbInsertHandler& handler, const char *xmlFileDir)
        : MyBase(handler, xmlFileDir, ".*bat.*\\.xml")
    {
    }

    std::shared_ptr<Section> CreateSection(const char* xmlPath) const;

private:
    void AddDescriptor(Section& nit, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor(Section& nit, uint16_t onId, xmlNodePtr& node, xmlChar* child) const;
    void AddTsDescriptor0x41(Section& bat, uint16_t tsId,
                             xmlNodePtr& node, xmlChar* child) const;
};

/**********************class EitXmlWrapper**********************/
template<typename Section>
class EitXmlWrapper: public XmlDataWrapper<Section>
{
public:
    typedef XmlDataWrapper<Section> MyBase;
    typedef EitXmlWrapper<Section> MyType;    

    EitXmlWrapper(DbInsertHandler& handler, const char *xmlFileDir)
        : MyBase(handler, xmlFileDir, ".*eit.*\\.xml")
    {
    }

    std::shared_ptr<Section> CreateSection(const char* xmlPath) const;
};

#endif