#ifndef _XmlDataWrapperDef_h_
#define _XmlDataWrapperDef_h_

#include <regex>
#include "DirMonitor.h"
#include "XmlDataWrapper.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

/**********************class XmlDataWrapper**********************/
template<typename Section>
XmlDataWrapper<Section>::XmlDataWrapper(DbInsertHandler& handler, const char *xmlFileDir, const char *xmlFileRegularExp)
    : MyBase(handler), xmlFileDir(xmlFileDir), xmlFileRegularExp(xmlFileRegularExp),
      dirMonitor(xmlFileDir, bind(&XmlDataWrapper::FileIoCompletionRoutine, this, _1))
{}

template<typename Section>
XmlDataWrapper<Section>::~XmlDataWrapper() 
{}   

template<typename Section>
void XmlDataWrapper<Section>::Start()
{
    /* linux api  : http://linux.die.net/man/7/inotify
        windows api: ReadDirectoryChangesW() or FileSystemWatcher component
        */
    dirMonitor.StartMonitoring();
}

template<typename Section>
void XmlDataWrapper<Section>::FileIoCompletionRoutine(const char *file)
{
    if (!regex_match(file, regex(xmlFileRegularExp)))
        return;
    
    string xmlPath = xmlFileDir + string("/") + string(file);
    CreateSection(file); 

    remove(xmlPath.c_str());
}

/**********************class NitXmlWrapper**********************/
template<typename Section>
void NitXmlWrapper<Section>::AddDescriptor(Section& nit, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        nit.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
    }   
}

template<typename Section>
void NitXmlWrapper<Section>::AddTsDescriptor(Section& nit, uint16_t tsId,
                                         xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        size_t size = strlen((const char*)data.get());
        shared_ptr<uchar_t> ascStr(new uchar_t[size/2]);
        ConvertStr2AscStr(data.get(), size, ascStr.get());

        nit.AddTsDescriptor(tsId, tag, ascStr.get(), size/2);
    }
}

template<typename Section>
void NitXmlWrapper<Section>::CreateSection(const char *file) const
{
    string xmlPath = xmlFileDir + string("/") + string(file);

    auto nit = std::make_shared<Section>();
    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    nit->SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Network[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    assert(nodes != nullptr && nodes->nodeNr == 1);
    
    node = nodes->nodeTab[0];
    nit->SetNetworkId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID"));
    nit->SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));
    shared_ptr<xmlChar> name = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
    nit->AddDescriptor(NetworkNameDescriptor::Tag, name.get(), strlen((const char*)name.get()));

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        AddDescriptor(*nit, node, (xmlChar*)"Descriptors");

        if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");
            uint32_t freq = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Frequency");
            uint16_t fecOuter = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"Fec_Outer");
            uchar_t  modulation = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Modulation");            
            uint32_t symbolRate = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Symbol_Rate");
            uint32_t fecInner = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Fec_Inner");

            nit->AddTs(tsId, onId);
            nit->AddTsDescriptor0x44(tsId, freq, fecOuter, modulation, symbolRate, fecInner);
            
            xmlNodePtr child = xmlFirstElementChild(node);
            AddTsDescriptor(*nit, tsId, child, (xmlChar*)"Descriptors");
        }
    }
    xmlCleanupParser();

    const char *ptr = find(file, file + strlen(file), '_') + 1;
    uint16_t netId = (uint16_t)strtol(file, nullptr, 10);
    uint16_t sn = (uint16_t)strtol(ptr, nullptr, 10);
    HandleDbInsert(netId, nit, sn);
}

/**********************class SdtXmlWrapper**********************/
template<typename Section>
void SdtXmlWrapper<Section>::AddService(Section& sdt, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    uint16_t serviceId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ServiceID");
    uchar_t  eitScheduleFlag = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"EIT_schedule_flag");
    uchar_t  eitPresentFollowingFlag = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"EIT_present_following_flag");
    uchar_t  runningStatus = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"running_status");
    uchar_t  freeCaMode = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"free_CA_mode");
    sdt.AddService(serviceId, eitScheduleFlag, eitPresentFollowingFlag, runningStatus, freeCaMode);
    
    shared_ptr<xmlChar> serviceName = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
    shared_ptr<xmlChar> providerName = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Provider_Name");
    uchar_t type = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Type");
    sdt.AddServiceDescriptor0x48(serviceId, type, providerName.get(), serviceName.get());

    for (xmlNodePtr cur = xmlFirstElementChild(xmlFirstElementChild(node)); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        sdt.AddServiceDescriptor(serviceId, tag, data.get(), strlen((const char*)data.get()));
    }
}

template<typename Section>
void SdtXmlWrapper<Section>::CreateSection(const char *file) const
{
    string xmlPath = xmlFileDir + string("/") + string(file);

    auto sdt = std::make_shared<Section>();
    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    sdt->SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    
    node = nodes->nodeTab[0];
    sdt->SetTsId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID"));
    sdt->SetOnId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID"));
    sdt->SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        AddService(*sdt, node, (xmlChar*)"Service");      
    }

    xmlCleanupParser();

    const char *ptr = find(file, file + strlen(file), '_') + 1;
    uint16_t netId = (uint16_t)strtol(file, nullptr, 10);
    uint16_t sn = (uint16_t)strtol(ptr, nullptr, 10);
    sdt->SetNetworkId(netId);
    HandleDbInsert(netId, sdt, sn);
}

/**********************class BatXmlWrapper**********************/
template<typename Section>
void BatXmlWrapper<Section>::AddDescriptor(Section& bat, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        bat.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
    }   
}

template<typename Section>
void BatXmlWrapper<Section>::AddTsDescriptor(Section& bat, uint16_t tsId,
                                         xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        size_t size = strlen((const char*)data.get());
        shared_ptr<uchar_t> ascStr(new uchar_t[size/2]);
        ConvertStr2AscStr(data.get(), size, ascStr.get());

        bat.AddTsDescriptor(tsId, tag, ascStr.get(), size/2);
    }
}

template<typename Section>
void BatXmlWrapper<Section>::AddTsDescriptor0x41(Section& bat, uint16_t tsId,
                                                 xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    list<pair<uint16_t, uchar_t>> serviceList;
    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uint16_t service = GetXmlAttrValue<uint16_t>(cur, (const xmlChar*)"ID");
        uchar_t type = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Type");
        serviceList.push_back(make_pair(service, type));
    }
    bat.AddTsDescriptor0x41(tsId, serviceList);
}

template<typename Section>
void BatXmlWrapper<Section>::CreateSection(const char *file) const
{
    string xmlPath = xmlFileDir + string("/") + string(file);

    auto bat = std::make_shared<Section>();
    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    bat->SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Bouquet[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    assert(nodes != nullptr && nodes->nodeNr == 1);

    node = nodes->nodeTab[0];
    bat->SetBouquetId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"BouquetID"));
    bat->SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));
    shared_ptr<xmlChar> name = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
    bat->AddDescriptor(BouquetNameDescriptor::Tag, name.get(), strlen((const char*)name.get()));

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        AddDescriptor(*bat, node, (xmlChar*)"Descriptors");

        if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

            bat->AddTs(tsId, onId);
            
            xmlNodePtr child = xmlFirstElementChild(node);
            AddTsDescriptor0x41(*bat, tsId, child, (xmlChar*)"ServiceList");

            child = xmlNextElementSibling(child);
            AddTsDescriptor(*bat, tsId, child, (xmlChar*)"Descriptors");
        }
    }

    xmlCleanupParser();
    
    const char *ptr = find(file, file + strlen(file), '_') + 1;
    uint16_t netId = (uint16_t)strtol(file, nullptr, 10);
    uint16_t sn = (uint16_t)strtol(ptr, nullptr, 10);
    bat->SetNetworkId(netId);
    HandleDbInsert(netId, bat, sn);
}

/**********************class EitXmlWrapper**********************/
template<typename Section>
void EitXmlWrapper<Section>::CreateSection(const char *file) const
{    
    string xmlPath = xmlFileDir + string("/") + string(file);

    auto eit = std::make_shared<Section>();
    shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    bat->SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    
    node = nodes->nodeTab[0];
    eit->SetTsId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID"));
    eit->SetOnId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID"));
    eit->SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));

    uint16_t serviceId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ServiceID");

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {      

        AddEvent(*sdt, node, (xmlChar*)"Service");      
    }

    xmlCleanupParser();
    
    const char *ptr = find(file, file + strlen(file), '_') + 1;
    uint16_t netId = (uint16_t)strtol(file, nullptr, 10);
    uint16_t sectionSn = (uint16_t)strtol(ptr, nullptr, 10);
    //HandleDbInsert(netId, sdt, sectionSn);
}

#endif