#include "SystemInclude.h"

#include "XmlDataWrapper.h"
#include "Gs9330Config.h"

using namespace std;

#if defined(_WIN32)

ConfigDataWrapper::ConfigDataWrapper()
{
}

void ConfigDataWrapper::ReadConfig(Config& config)
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

    shared_ptr<xmlChar> regionCode = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"ReceivDir");
    config.regionCode = (char*)regionCode.get();
    
    shared_ptr<xmlChar> addr = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"TsDstIP");
    config.tsDstIp = inet_addr((const char *)addr.get());
    config.tsDstPort = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TsDstPort");
    config.tsInterval = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"TsInterval");
}

#endif