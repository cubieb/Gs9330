#ifndef _SiTableXmlWrapperInterface_h_
#define _SiTableXmlWrapperInterface_h_

#include "Include/Foundation/SystemInclude.h"
#pragma warning(push)
#pragma warning(disable:702)   //disable warning caused by ACE library.
#pragma warning(disable:4251)  //disable warning caused by ACE library.
#pragma warning(disable:4996)  //disable warning caused by ACE library.
#include "ace/OS.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/Deleter.h"
#include "Include/Foundation/XmlHelper.h"

/**********************class SiTableXmlWrapperInterface**********************/
template<typename TsPacket>
class SiTableXmlWrapperInterface
{
public:
    SiTableXmlWrapperInterface() {};
    virtual ~SiTableXmlWrapperInterface() {};

    //tableId and keys are used to record operation history */
    virtual std::error_code Select(TsPacket &tsPacket, const char *xmlPath, 
                                   TableId &tableId, std::list<SiTableKey> &keys) = 0;
};

/**********************class NitXmlWrapper**********************/
template<typename TsPacket, typename SiTable>
class BatXmlWrapper: public SiTableXmlWrapperInterface<TsPacket>
{
public:
    BatXmlWrapper() {};
    virtual ~BatXmlWrapper() {};

    std::error_code Select(TsPacket &tsPacket, const char *xmlPath, 
                           TableId &tableId, std::list<SiTableKey> &keys)
    {
        cout << "Reading " << xmlPath << endl;
        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            return  make_error_code(std::errc::no_such_file_or_directory);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Bouquet[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            BouquetId bouquetId = GetXmlAttrValue<BouquetId>(node, (const xmlChar*)"BouquetID");
            Version versionNumber = GetXmlAttrValue<Version>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateBatInstance(tableId, bouquetId, versionNumber);
            keys.push_back((SiTableKey)bouquetId);

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

        xmlCleanupParser(); 
        return std::error_code();
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

/**********************class EitXmlWrapper**********************/
template<typename TsPacket, typename SiTable>
class EitXmlWrapper: public SiTableXmlWrapperInterface<TsPacket>
{
public:
    EitXmlWrapper() {};
    virtual ~EitXmlWrapper() {};

    std::error_code Select(TsPacket &tsPacket, const char *xmlPath, 
                           TableId &tableId, std::list<SiTableKey> &keys)
    {
        cout << "Reading " << xmlPath << endl;        
        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            return  make_error_code(std::errc::no_such_file_or_directory);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");
        if (tableId == 0x4E)
            tableId = 0x50;
        else if (tableId == 0x4F)
            tableId = 0x60;

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }
    
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];

            TsId tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            NetId onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            ServiceId serviceId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ServiceID");

            SiTable *siTable = SiTable::CreateEitInstance(tableId, serviceId, versionNumber, tsId, onId);
            SiTableKey key = (tsId << 16) | serviceId;
            keys.push_back(key);

            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                AddEvent(*siTable, node);
            }
            tsPacket.AddSiTable(siTable);
        }

        xmlCleanupParser();
        return std::error_code();
    }

    void AddEvent(SiTable& siTable, xmlNodePtr& node) const
    {
        uint16_t eventId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"EventID");
        SharedXmlChar startTime = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"StartTime");
        uint32_t duration = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Duration");
        uint16_t  runningStatus = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"running_status");
        uint16_t  freeCaMode = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"free_CA_mode");

        siTable.AddEvent(eventId, (char*)startTime.get(), (time_t)duration, runningStatus, freeCaMode);
    
        for (xmlNodePtr cur = xmlFirstElementChild(xmlFirstElementChild(node)); 
             cur != nullptr; 
             cur = xmlNextElementSibling(cur))
        {
            uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
            //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
            std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

            siTable.AddEventDescriptor(eventId, data);
        }
    }
};

/**********************class NitXmlWrapper**********************/
template<typename TsPacket, typename SiTable>
class NitXmlWrapper: public SiTableXmlWrapperInterface<TsPacket>
{
public:
    NitXmlWrapper() {};
    virtual ~NitXmlWrapper() {};

    std::error_code Select(TsPacket &tsPacket, const char *xmlPath, 
                           TableId &tableId, std::list<SiTableKey> &keys)
    {
        cout << "Reading " << xmlPath << endl;
        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            return  make_error_code(std::errc::no_such_file_or_directory);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Network[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }
        
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            NetId networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateNitInstance(tableId, networkId, versionNumber);
            keys.push_back((SiTableKey)networkId);

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

        xmlCleanupParser(); 
        return std::error_code();
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

/**********************class SdtXmlWrapper**********************/
template<typename TsPacket, typename SiTable>
class SdtXmlWrapper: public SiTableXmlWrapperInterface<TsPacket>
{
public:
    SdtXmlWrapper() {};
    virtual ~SdtXmlWrapper() {};

    std::error_code Select(TsPacket &tsPacket, const char *xmlPath, 
                           TableId &tableId, std::list<SiTableKey> &keys)
    {
        cout << "Reading " << xmlPath << endl;
        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            return  make_error_code(std::errc::no_such_file_or_directory);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }
        
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            TsId tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            NetId onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateSdtInstance(tableId, tsId, versionNumber, onId);
            keys.push_back((SiTableKey)tsId);

            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                AddService(*siTable, node, (xmlChar*)"Service");      
            }

            tsPacket.AddSiTable(siTable);
        } //for (int i = 0; i < nodes->nodeNr; ++i)

        xmlCleanupParser();
        return std::error_code();
    }

    void AddService(SiTable& siTable, xmlNodePtr& node, xmlChar* child) const
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
        siTable.AddService(serviceId, eitScheduleFlag, eitPresentFollowingFlag, runningStatus, freeCaMode);

        for (xmlNodePtr cur = xmlFirstElementChild(xmlFirstElementChild(node)); 
            cur != nullptr; 
            cur = xmlNextElementSibling(cur))
        {
            uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
            //SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
            std::string data = GetXmlAttrValue<std::string>(cur, (const xmlChar*)"Data");

            siTable.AddServiceDescriptor(serviceId, data);
        }
    }
};

/**********************class SiTableXmlWrapperRepository**********************/
template<typename TsPacket>
class SiTableXmlWrapperRepository
{
public:
    SiTableXmlWrapperRepository() {}
    ~SiTableXmlWrapperRepository() 
    {
        std::map<std::string, SiTableXmlWrapperInterface<TsPacket> *>::iterator iter;
        for (iter = wrappers.begin(); iter != wrappers.end(); ++iter)
        {
            delete iter->second;
        }
    }

    SiTableXmlWrapperInterface<TsPacket>& GetWrapperInstance(std::string &type)
    {
        std::map<std::string, SiTableXmlWrapperInterface<TsPacket> *>::iterator iter;
        iter = wrappers.find(type);
        assert(iter != wrappers.end());
        return *iter->second;
    }

    static SiTableXmlWrapperRepository& GetInstance()
    {
        static SiTableXmlWrapperRepository instance;
        return instance;
    }

    void Register(std::string &type, SiTableXmlWrapperInterface<TsPacket> * wrapper)
    {
        wrappers.insert(make_pair(type, wrapper));
    }

private:
    std::map<std::string, SiTableXmlWrapperInterface<TsPacket> *> wrappers;
};

template<typename TsPacket>
class SiTableXmlWrapperAutoRegisterSuite
{
public:
    SiTableXmlWrapperAutoRegisterSuite(std::string &type,  
                                       SiTableXmlWrapperInterface<TsPacket> * wrapper)
    {
        SiTableXmlWrapperRepository<TsPacket>& repository = SiTableXmlWrapperRepository<TsPacket>::GetInstance();
        repository.Register(type, wrapper);
    }
};

#pragma warning(pop)
#endif