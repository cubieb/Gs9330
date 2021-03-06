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
template<typename SiTable>
class SiTableXmlWrapperInterface
{
public:
    SiTableXmlWrapperInterface() {};
    virtual ~SiTableXmlWrapperInterface() {};

    //tableId and keys are used to record operation history */
    virtual std::list<SiTable*> Select(const char *xmlPath) const = 0;
};

/**********************class NitXmlWrapper**********************/
template<typename SiTable>
class BatXmlWrapper: public SiTableXmlWrapperInterface<SiTable>
{
public:
    BatXmlWrapper() {};
    virtual ~BatXmlWrapper() {};

    std::list<SiTable*> Select(const char *xmlPath) const
    {
        cout << "Reading " << xmlPath << endl;
        std::list<SiTable*> siTables;

        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        TableId tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Bouquet[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            BouquetId bouquetId = GetXmlAttrValue<BouquetId>(node, (const xmlChar*)"BouquetID");
            Version versionNumber = GetXmlAttrValue<Version>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateBatInstance(tableId, bouquetId, versionNumber);

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

            siTables.push_back(siTable);
        }

        xmlCleanupParser(); 
        return siTables;
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
template<typename SiTable>
class EitXmlWrapper: public SiTableXmlWrapperInterface<SiTable>
{
public:
    EitXmlWrapper() {};
    virtual ~EitXmlWrapper() {};

    std::list<SiTable*> Select(const char *xmlPath) const
    {
        cout << "Reading " << xmlPath << endl; 
        std::list<SiTable*> siTables;

        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        TableId tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");
        if (tableId == 0x4E)
            tableId = 0x50;
        else if (tableId == 0x4F)
            tableId = 0x60;

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }
    
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];

            TsId tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            NetId onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            ServiceId serviceId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ServiceID");

            SiTable *siTable = SiTable::CreateEitInstance(tableId, serviceId, versionNumber, tsId, onId);
            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                AddEvent(*siTable, node);
            }

            siTables.push_back(siTable);
        }

        xmlCleanupParser();
        return siTables;
    }

private:
    void AddEvent(SiTable& siTable, xmlNodePtr& node) const
    {
        uint16_t eventId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"EventID");
        SharedXmlChar startTime = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"StartTime");
        uint32_t bcd = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Duration");
        time_t   duration = (bcd / 10000 * 3600) + ((bcd % 10000) / 100 * 60) + (bcd % 100); 
        uint16_t  runningStatus = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"running_status");
        uint16_t  freeCaMode = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"free_CA_mode");

        siTable.AddEvent(eventId, (char*)startTime.get(), duration, runningStatus, freeCaMode);
    
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
template<typename SiTable>
class NitXmlWrapper: public SiTableXmlWrapperInterface<SiTable>
{
public:
    NitXmlWrapper() {};
    virtual ~NitXmlWrapper() {};

    std::list<SiTable*> Select(const char *xmlPath) const
    {
        cout << "Reading " << xmlPath << endl;
        std::list<SiTable*> siTables;

        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        TableId tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");

        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Network[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }
        
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            NetId networkId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateNitInstance(tableId, networkId, versionNumber);

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
        
            siTables.push_back(siTable);
        }

        xmlCleanupParser(); 
        return siTables;
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
template<typename SiTable>
class SdtXmlWrapper: public SiTableXmlWrapperInterface<SiTable>
{
public:
    SdtXmlWrapper() {};
    virtual ~SdtXmlWrapper() {};

    std::list<SiTable*> Select(const char *xmlPath) const
    {
        cout << "Reading " << xmlPath << endl;
        std::list<SiTable*> siTables;

        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        TableId tableId = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID");
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }

        xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            errstrm << "Error when reading " << xmlPath << endl;
            return  siTables;
        }
        
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            TsId tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            NetId onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");
            Version versionNumber = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version");
            SiTable *siTable = SiTable::CreateSdtInstance(tableId, tsId, versionNumber, onId);

            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                AddService(*siTable, node, (xmlChar*)"Service");      
            }

            siTables.push_back(siTable);
        } //for (int i = 0; i < nodes->nodeNr; ++i)

        xmlCleanupParser(); 
        return siTables;
    }

private:
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
template<typename SiTable>
class SiTableXmlWrapperRepository
{
public:
    SiTableXmlWrapperRepository() {}
    ~SiTableXmlWrapperRepository() 
    {
        std::map<std::string, SiTableXmlWrapperInterface<SiTable> *>::iterator iter;
        for (iter = wrappers.begin(); iter != wrappers.end(); ++iter)
        {
            delete iter->second;
        }
    }

    SiTableXmlWrapperInterface<SiTable>& GetWrapperInstance(std::string &type)
    {
        std::map<std::string, SiTableXmlWrapperInterface<SiTable> *>::iterator iter;
        iter = wrappers.find(type);
        assert(iter != wrappers.end());
        return *iter->second;
    }

    static SiTableXmlWrapperRepository& GetInstance()
    {
        static SiTableXmlWrapperRepository instance;
        return instance;
    }

    void Register(std::string &type, SiTableXmlWrapperInterface<SiTable> * wrapper)
    {
        wrappers.insert(make_pair(type, wrapper));
    }

private:
    std::map<std::string, SiTableXmlWrapperInterface<SiTable> *> wrappers;
};

template<typename SiTable>
class SiTableXmlWrapperAutoRegisterSuite
{
public:
    SiTableXmlWrapperAutoRegisterSuite(std::string &type,  
                                       SiTableXmlWrapperInterface<SiTable> * wrapper)
    {
        SiTableXmlWrapperRepository<SiTable>& repository = SiTableXmlWrapperRepository<SiTable>::GetInstance();
        repository.Register(type, wrapper);
    }
};

#pragma warning(pop)
#endif