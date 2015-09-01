#ifndef _Gs9330Config_h_
#define _Gs9330Config_h_

#include "SystemInclude.h"
/**********************class NetworkIpConfig**********************/
struct NetworkIpConfig
{
    struct Entry
    {
        uint16_t    networkId;
        uint32_t    tsDstIp;
        uint16_t    tsDstPort;
        Entry(uint16_t networkId, uint32_t tsDstIp, uint16_t tsDstPort)
            : networkId(networkId), tsDstIp(tsDstIp), tsDstPort(tsDstPort)
        {}
    };
    std::list<Entry> entries;
    uint32_t         tsInterval;  /* upd packet interval in mini seconds */
};

/**********************class NetworkIpConfigWrapper**********************/
class NetworkIpConfigWrapper
{
public:
    NetworkIpConfigWrapper();
    void ReadConfig(NetworkIpConfig&);
};

/**********************class NetworkRelationConfig**********************/
struct NetworkRelationConfig
{
    //uint16_t networkId;
    //uint16_t parentNetworkId;
    std::map<uint16_t, uint16_t> relation;
    bool IsChildNetwork(uint16_t ancestor, uint16_t offspring)
    {
        if (ancestor == offspring)
            return true;

        std::map<uint16_t, uint16_t>::iterator iter;  
        for (auto iter = relation.find(offspring); 
             iter != relation.end();
             iter = relation.find(iter->second))
        {
            if (iter->first == ancestor)
                return true;
        }

        return false;
    }

    void Clear()
    {
        relation.clear();
    }
};

/**********************class NetworkRelationConfigWrapper**********************/
class NetworkRelationConfigWrapper
{
public:
    NetworkRelationConfigWrapper(const char *xmlFile);
    void ReadConfig(NetworkRelationConfig&);

private:
    std::string xmlFile;
};

/**********************class XmlConfig**********************/
struct XmlConfig
{
    std::string xmlDir;
};

/**********************class XmlConfigWrapper**********************/
class XmlConfigWrapper
{
public:
    XmlConfigWrapper();
    void ReadConfig(XmlConfig&);
};

#endif