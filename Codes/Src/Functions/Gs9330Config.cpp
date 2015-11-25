#include "SystemInclude.h"

#include "XmlDataWrapper.h"
#include "Gs9330Config.h"

using namespace std;

#if defined(_WIN32)
#include "io.h"
/**********************class TransmitConfig**********************/
TransmitConfig::TransmitConfig()
{
    char xmlPath[MAX_PATH];
    DWORD dword = ExpandEnvironmentStrings("%APPDATA%", xmlPath, MAX_PATH);
    assert(dword != 0);

    strcat(xmlPath, "\\Gospell\\Gs9330Transmit.xml");
    if (_access(xmlPath, 0) == -1)
    {
        strcpy(xmlPath, "Gs9330Transmit.xml");
    }

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    nitActualInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"NitActualInterval");
    nitOtherInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"NitOtherInterval");
    batInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"BatInterval");
    sdtActualInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"SdtActualInterval");
    sdtOtherInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"SdtOtherInterval");
    eit4EInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Eit4EInterval");
    eit4FInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Eit4FInterval");
    eit50Interval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Eit50Interval");
    eit60Interval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Eit60Interval");
    assert(nitActualInterval != 0);
    assert(nitOtherInterval != 0);
    assert(batInterval != 0);
    assert(sdtOtherInterval != 0);
    assert(sdtOtherInterval != 0);
    assert(eit4EInterval != 0);    
    assert(eit4FInterval != 0);
    assert(eit50Interval != 0);
    assert(eit60Interval != 0);

    shared_ptr<xmlChar> dir = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"TsFilesDir");
    tsFilesDir = (char*)dir.get();

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    //xpathExpr cant be "/Root/Network[*], because Network has no child node."
    xmlChar *xpathExpr = (xmlChar*)"/Root/Network";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i)
    {
        xmlNodePtr node = nodes->nodeTab[i];

        uint16_t networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID");
        shared_ptr<xmlChar> dstIp = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"DstIP");
        uint16_t dstPort = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"DstPort");

        netAddresses.push_back(make_shared<NetworkIdAddress>(networkId, (const char *)dstIp.get(), dstPort));
    }

    ftpInterval = 60;
    xmlCleanupParser();
}

/**********************class XmlConfig**********************/
XmlConfig::XmlConfig()
{    
    char xmlPath[MAX_PATH] = {'\0'};
    DWORD dword = ExpandEnvironmentStrings("%APPDATA%", xmlPath, MAX_PATH);
    assert(dword != 0);

    strcat(xmlPath, "\\Gospell\\Gs9330SoapClient.xml");

    if (_access(xmlPath, 0) == -1)
    {
        strcpy(xmlPath, "Gs9330SoapClient.xml");
    }

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
    assert(doc != nullptr);
    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    node = xmlFirstElementChild(node);  /* ChildXML */
    assert(strcmp((const char *)node->name, "ChildXML") == 0);

    shared_ptr<xmlChar> dir = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"ReceivDir");
    xmlDir = (char*)dir.get();
}

/**********************class NetworkRelationConfig**********************/
NetworkRelationConfig::NetworkRelationConfig()
{}

NetworkRelationConfig::NetworkRelationConfig(const char *xmlFile)
{
    relations.clear();

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFile), XmlDocDeleter());
    assert(doc != nullptr);
    
    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    //xpathExpr cant be "/Root/NetNode[*], because Network has no child node."
    xmlChar *xpathExpr = (xmlChar*)"/Root/NetNode";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;

    for (int i = 0; i < nodes->nodeNr; ++i)
    {
        xmlNodePtr node = nodes->nodeTab[i];
        uint16_t networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"NetID");
        uint16_t tsDstPort = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"FatherID");
        relations.insert(make_pair(networkId, tsDstPort));
    }

    xmlCleanupParser();
}

bool NetworkRelationConfig::IsChildNetwork(uint16_t ancestor, uint16_t offspring)
{
    if (ancestor == offspring)
        return true;

    std::map<uint16_t, uint16_t>::iterator iter;  
    for (auto iter = relations.find(offspring); 
            iter != relations.end();
            iter = relations.find(iter->second))
    {
        if (iter->first == ancestor)
            return true;
    }

    return false;
}
#endif