#ifndef _XmlDataWrapperDef_h_
#define _XmlDataWrapperDef_h_

#include <regex>
#include "DirMonitor.h"
#include "XmlDataWrapper.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

std::pair<shared_ptr<char>, size_t> ConvertUtf8ToGb2312(const std::shared_ptr<xmlChar>& str)
{
    size_t wcharNum = MultiByteToWideChar(CP_UTF8, 0, 
        (char*)str.get(),  /* lpMultiByteStr, Pointer to the character string to convert. */
        -1,                /* cbMultiByte, can be -1, if the string is null-terminated.*/
        NULL, 0);
    if (wcharNum == 1)
    {
        /* just a '\0' */
        shared_ptr<char> gb2312(new char[1], CharDeleter());
        gb2312.get()[0] = '\0';
        return make_pair(gb2312, 1);
    }
    size_t strBytes = sizeof(WCHAR) * wcharNum; /* 0x13 + gb2312-string + '\0' */
    shared_ptr<char> gb2312(new char[strBytes], CharDeleter());
    gb2312.get()[0] = 0x13;
    ConvertUtf8ToGb2312(gb2312.get() + 1, strBytes - 1, (char*)str.get());

    return make_pair(gb2312, strBytes);
}

/**********************class XmlDataWrapper**********************/
template<typename Section>
XmlDataWrapper<Section>::XmlDataWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler,
        const char *xmlFileDir, const char *xmlFileRegularExp)
    : MyBase(insertHandler, deleteHandler), xmlFileDir(xmlFileDir), xmlFileRegularExp(xmlFileRegularExp)
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
    DirMonitor& dirMonitor = DirMonitor::GetInstance();
    dirMonitor.AddDir(xmlFileDir.c_str(), 
        bind(&XmlDataWrapper::HandleDbInsert, this, _1),
        bind(&XmlDataWrapper::HandleDbDelete, this, _1));
}

//DirMonitor -> XmlDataWrapper<Section>::HandleDbInsert()
template<typename Section>
void XmlDataWrapper<Section>::HandleDbInsert(const char *file)
{
    if (!regex_match(file, regex(xmlFileRegularExp)))
        return;
    
    string xmlPath = xmlFileDir + string("/") + string(file);
    CreateSection(file);   //->NotifyDbInsert()
}

//DirMonitor -> XmlDataWrapper<Section>::HandleDbDelete()
template<typename Section>
void XmlDataWrapper<Section>::HandleDbDelete(const char *file)
{
    if (!regex_match(file, regex(xmlFileRegularExp)))
        return;
    
    NotifyDbDelete(xmlFileRegularExp.substr(2,3).c_str(), file);
}

/**********************class NitXmlWrapper**********************/
template<typename Section>
NitXmlWrapper<Section>::NitXmlWrapper(DbInsertHandler& insertHandler, 
                                      DbDeleteHandler& deleteHandler, 
                                      const char *xmlFileDir)
    : MyBase(insertHandler, deleteHandler, xmlFileDir, ".*nit.*\\.xml")
{
}

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

    auto nit = std::make_shared<Section>(file);
    shared_ptr<xmlDoc> doc;
    for (int i = 0; i < 10 && doc == nullptr; ++i)
    {
        if (i != 0)
            SleepEx(10, true);
        doc.reset(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    }
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
    auto nameGb2312 = ConvertUtf8ToGb2312(name);
    nit->AddDescriptor(NetworkNameDescriptor::Tag, (uchar_t*)nameGb2312.first.get(), nameGb2312.second);

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
    NotifyDbInsert(nit);
}

/**********************class SdtXmlWrapper**********************/
template<typename Section>
SdtXmlWrapper<Section>::SdtXmlWrapper(DbInsertHandler& insertHandler, 
                                      DbDeleteHandler& deleteHandler, 
                                      const char *xmlFileDir)
    : MyBase(insertHandler, deleteHandler, xmlFileDir, ".*sdt.*\\.xml")
{}

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
    auto serviceNameGb2312 = ConvertUtf8ToGb2312(serviceName);
    auto providerNameGb2312 = ConvertUtf8ToGb2312(providerName);
    uchar_t type = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Type");

    sdt.AddServiceDescriptor0x48(serviceId, type, 
        (uchar_t*)providerNameGb2312.first.get(), 
        (uchar_t*)serviceNameGb2312.first.get());

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

    auto sdt = std::make_shared<Section>(file);
    shared_ptr<xmlDoc> doc;
    for (int i = 0; i < 10 && doc == nullptr; ++i)
    {
        if (i != 0)
            SleepEx(10, true);
        doc.reset(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    }
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
    sdt->SetNetworkId(netId);
    NotifyDbInsert(sdt);
}

/**********************class BatXmlWrapper**********************/
template<typename Section>
BatXmlWrapper<Section>::BatXmlWrapper(DbInsertHandler& insertHandler, 
                                      DbDeleteHandler& deleteHandler, 
                                      const char *xmlFileDir)
    : MyBase(insertHandler, deleteHandler, xmlFileDir, ".*bat.*\\.xml")
{
}

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

    auto bat = std::make_shared<Section>(file);
    shared_ptr<xmlDoc> doc;
    for (int i = 0; i < 10 && doc == nullptr; ++i)
    {
        if (i != 0)
            SleepEx(10, true);
        doc.reset(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    }
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
    auto nameGb2312 = ConvertUtf8ToGb2312(name);
    bat->AddDescriptor(BouquetNameDescriptor::Tag, (uchar_t*)nameGb2312.first.get(), nameGb2312.second);    

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
    bat->SetNetworkId(netId);   
    NotifyDbInsert(bat);
}

/**********************class EitXmlWrapper**********************/
template<typename Section>
EitXmlWrapper<Section>::EitXmlWrapper(DbInsertHandler& insertHandler, 
                                      DbDeleteHandler& deleteHandler, 
                                      const char *xmlFileDir)
    : MyBase(insertHandler, deleteHandler, xmlFileDir, ".*eit.*\\.xml")
{
}

template<typename Section>
void EitXmlWrapper<Section>::CreateSection(const char *file) const
{    
    string xmlPath = xmlFileDir + string("/") + string(file);
    
    shared_ptr<xmlDoc> doc;
    for (int i = 0; i < 10 && doc == nullptr; ++i)
    {
        if (i != 0)
            SleepEx(10, true);
        doc.reset(xmlParseFile(xmlPath.c_str()), XmlDocDeleter());
    }
    assert(doc != nullptr);

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    uchar_t tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    
    for (int i = 0; i < nodes->nodeNr; ++i)
    {
        auto eit = std::make_shared<Section>(file);
        eit->SetTableId(tableId);

        node = nodes->nodeTab[i];
        eit->SetTsId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID"));
        eit->SetOnId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID"));
        eit->SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));
        eit->SetServiceId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ServiceID"));

        for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
        {
            AddEvent(*eit, node, (xmlChar*)"Event");
        }
    
        const char *ptr = find(file, file + strlen(file), '_') + 1;
        uint16_t netId = (uint16_t)strtol(file, nullptr, 10);
        eit->SetNetworkId(netId);
        NotifyDbInsert(eit);
    }

    xmlCleanupParser();
}

template<typename Section>
void EitXmlWrapper<Section>::AddEvent(Section& eit, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    uint16_t eventId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"EventID");
    SharedXmlChar startTime = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"StartTime");
    uint32_t duration = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Duration");
    uint16_t  runningStatus = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"running_status");
    uint16_t  freeCaMode = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"free_CA_mode");

    eit.AddEvent(eventId, (char*)startTime.get(), (time_t)duration, runningStatus, freeCaMode);

    for (xmlNodePtr cur = xmlFirstElementChild(xmlFirstElementChild(node)); 
         cur != nullptr; 
         cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        eit.AddEventDescriptor(eventId, tag, data.get(), strlen((const char*)data.get()));
    }
}

#endif