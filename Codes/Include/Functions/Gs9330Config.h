#ifndef _Gs9330Config_h_
#define _Gs9330Config_h_

#include "SystemInclude.h"

/**********************class NetworkIpEntry**********************/
class NetworkIdAddress
{
public:
    NetworkIdAddress(uint16_t networkId, uint32_t tsDstIp, uint16_t tsDstPort)
        : networkId(networkId), tsDstIp(tsDstIp), tsDstPort(tsDstPort)
    {}

public:
    uint16_t    networkId;
    uint32_t    tsDstIp;
    uint16_t    tsDstPort;
};

/**********************class TransmitConfig**********************/
class TransmitConfig
{
public:
    TransmitConfig();
    void AddNetAddr(uint16_t networkId, uint32_t tsDstIp, uint16_t tsDstPort);

public:
    /* upd packet interval in seconds */
    uint32_t   nitActualInterval;
    uint32_t   nitOtherInterval;
    uint32_t   batInterval;
    uint32_t   sdtActualInterval;
    uint32_t   sdtOtherInterval;
    uint32_t   eit4EInterval;
    uint32_t   eit4FInterval;
    uint32_t   eit50Interval;
    uint32_t   eit60Interval;
    
    std::list<std::shared_ptr<NetworkIdAddress>> netAddresses;
};

/**********************class XmlConfig**********************/
class XmlConfig
{
public:
    XmlConfig();

public:
    std::string xmlDir;
};

/**********************class NetworkRelationConfig**********************/
class NetworkRelationConfig
{
public:
    NetworkRelationConfig(const char *xmlFile);
    
    bool IsChildNetwork(uint16_t ancestor, uint16_t offspring);

public:
    //map<networkId, parentNetworkId>
    std::map<uint16_t, uint16_t> relations;
};

#endif