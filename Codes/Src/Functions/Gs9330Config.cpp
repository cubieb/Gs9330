#include "SystemInclude.h"

#include "XmlDataWrapper.h"
#include "Gs9330Config.h"

using namespace std;

#if defined(_WIN32)

/**********************class NetworkIpConfig**********************/
NetworkIpConfigWrapper::NetworkIpConfigWrapper()
{
}

void NetworkIpConfigWrapper::ReadConfig(NetworkIpConfig& config)
{
    char xmlPath[MAX_PATH];
    DWORD dword = ExpandEnvironmentStrings("%APPDATA%", xmlPath, MAX_PATH);
    assert(dword != 0);

    strcat(xmlPath, "\\Gospell\\Gs9330Netwok.xml");

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    config.tsInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"PacketInterval");

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Network";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i)
    {
        xmlNodePtr node = nodes->nodeTab[i];

        uint16_t networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID");
        shared_ptr<xmlChar> addr = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"TsDstIP");
        uint32_t tsDstIp = inet_addr((const char *)addr.get());
        
        uint16_t tsDstPort = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TsDstPort");
        config.entries.push_back(NetworkIpConfig::Entry(networkId, tsDstIp, tsDstPort));
    }

    xmlCleanupParser();
}

/**********************class NetworkRelationConfig**********************/
NetworkRelationConfigWrapper::NetworkRelationConfigWrapper(const char *xmlFile)
    : xmlFile(xmlFile)
{}

void NetworkRelationConfigWrapper::ReadConfig(NetworkRelationConfig& config)
{
    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFile.c_str()), XmlDocDeleter());
    assert(doc != nullptr);
    config.Clear();

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/NetNode";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i)
    {
        xmlNodePtr node = nodes->nodeTab[i];
        uint16_t networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"NetID");
        uint16_t tsDstPort = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"FatherID");
        config.relation.insert(make_pair(networkId, tsDstPort));
    }

    xmlCleanupParser();
}

/**********************class XmlConfigWrapper**********************/
XmlConfigWrapper::XmlConfigWrapper()
{
}

void XmlConfigWrapper::ReadConfig(XmlConfig& config)
{
    char xmlPath[MAX_PATH];
    DWORD dword = ExpandEnvironmentStrings("%APPDATA%", xmlPath, MAX_PATH);
    assert(dword != 0);

    strcat(xmlPath, "\\Gospell\\Gs9330SoapClient.xml");

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
    assert(doc != nullptr);
    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    node = xmlFirstElementChild(node);  /* ChildXML */
    assert(strcmp((const char *)node->name, "ChildXML") == 0);

    shared_ptr<xmlChar> xmlDir = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"ReceivDir");
    config.xmlDir = (char*)xmlDir.get();
}

#endif