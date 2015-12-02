#ifndef _XmlDataWrapperInl_h_
#define _XmlDataWrapperInl_h_

#include <regex>
#include <io.h> 
#include "Debug.h"
#include "DirMonitor.h"
#include "XmlDataWrapper.h" 

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

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
    string path = xmlFileDir + string("\\*.xml");
    _finddata_t fileInfo;  
    long handle = _findfirst(path.c_str(), &fileInfo); 
    if (handle != -1) 
    {
        do 
        {  
            dbgstrm << fileInfo.name << endl; 
            if (regex_match(fileInfo.name, regex(xmlFileRegularExp)))
            {
                CreateSection(fileInfo.name);   //->NotifyDbInsert()
            }
        } while (_findnext(handle, &fileInfo) == 0); 
    } 

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

/* Example XML snippet:
    <Descriptors Sum="1">     # <--  node
        <Item Tag="0x40" Data="04ÉòÑôÊÐÍø"/>
    </Descriptors>
*/
template<typename Section>
void NitXmlWrapper<Section>::AddDescriptor(Section& nit, xmlNodePtr& node) const
{
    assert(xmlStrcmp(node->name, (const xmlChar*)"Descriptors") == 0);
    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

        nit.AddDescriptor(data);
    }   
}

/* Example XML snippet:
    <Descriptors Sum="1">     # <--  node
        <Item Tag="0x5F" Data="000021"/>
    </Descriptors>
*/
template<typename Section>
void NitXmlWrapper<Section>::AddTsDescriptor(Section& nit, uint16_t tsId, xmlNodePtr& node) const
{
    assert(xmlStrcmp(node->name, (const xmlChar*)"Descriptors") == 0);

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

        nit.AddTsDescriptor(tsId, data);
    }
}

template<typename Section>
void NitXmlWrapper<Section>::CreateSection(const char *file) const
{
    string xmlPath = xmlFileDir + string("/") + string(file);
    cout << "Reading " << file << endl;

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

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        if (xmlStrcmp(node->name, (xmlChar*)"Descriptors") == 0)
        {
            AddDescriptor(*nit, node);
        }
        else if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

            nit->AddTs(tsId, onId);
            
            xmlNodePtr child = xmlFirstElementChild(node);
            AddTsDescriptor(*nit, tsId, child);
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

    //shared_ptr<xmlChar> str = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"LCN");
    //uint16_t lcnFlag = (strlen((char*)str.get()) == 0) ? 0 : 1;
    //uint16_t lcn = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"LCN");

    //str = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Volume");
    //uchar_t vcFlag = (strlen((char*)str.get()) == 0) ? 0 : 1;
    //uchar_t volumeCompensation = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Volume");
    //
    //if (lcnFlag != 0 || vcFlag != 0)
    //{
    //    uchar_t descriptor0x83[3];
    //    Write16(&descriptor0x83[0], (lcnFlag << 15) | lcn);
    //    Write8(&descriptor0x83[2], (vcFlag << 7) | volumeCompensation);
    //    sdt.AddServiceDescriptor(serviceId, UserdefinedDscriptor83::Tag, descriptor0x83, 3);
    //}
    
    for (xmlNodePtr cur = xmlFirstElementChild(xmlFirstElementChild(node)); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

        sdt.AddServiceDescriptor(serviceId, data);
    }
}

template<typename Section>
void SdtXmlWrapper<Section>::CreateSection(const char *file) const
{
    string xmlPath = xmlFileDir + string("/") + string(file);
    cout << "Reading " << file << endl;

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
void BatXmlWrapper<Section>::AddDescriptor(Section& bat, xmlNodePtr& node) const
{
    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

        bat.AddDescriptor(data);
    }   
}

template<typename Section>
void BatXmlWrapper<Section>::AddTsDescriptor(Section& bat, uint16_t tsId,
                                             xmlNodePtr& node) const
{
    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");
        
        bat.AddTsDescriptor(tsId, data);
    }
}

template<typename Section>
void BatXmlWrapper<Section>::CreateSection(const char *file) const
{
    string xmlPath = xmlFileDir + string("/") + string(file);
    cout << "Reading " << file << endl;

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

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        if (xmlStrcmp(node->name, (xmlChar*)"Descriptors") == 0)
        {
            AddDescriptor(*bat, node);
        }
        else if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

            bat->AddTs(tsId, onId);
            xmlNodePtr child = xmlFirstElementChild(node);
            AddTsDescriptor(*bat, tsId, child);
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
    cout << "Reading " << file << endl;
    
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
    if (tableId == 0x4E)
        tableId = 0x50;
    else if (tableId == 0x4F)
        tableId = 0x60;

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
            AddEvent(*eit, node);
        }
    
        const char *ptr = find(file, file + strlen(file), '_') + 1;
        uint16_t netId = (uint16_t)strtol(file, nullptr, 10);
        eit->SetNetworkId(netId);
        NotifyDbInsert(eit);
    }

    xmlCleanupParser();
}

template<typename Section>
void EitXmlWrapper<Section>::AddEvent(Section& eit, xmlNodePtr& node) const
{
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
        //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

        eit.AddEventDescriptor(eventId, data);
    }
}

#endif