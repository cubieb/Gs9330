#ifndef _Gs9330Config_h_
#define _Gs9330Config_h_

#include "SystemInclude.h"

/**********************class NetworkIpEntry**********************/
class NetworkIdAddress
{
public:
    NetworkIdAddress(uint16_t networkId, const char *dstIp, uint16_t dstPort)
        : networkId(networkId), dstIp(dstIp), dstPort(dstPort)
    {}

public:
    uint16_t    networkId;
    std::string dstIp;
    uint16_t    dstPort;
};

struct CmpNetworkIdAddressId: public std::binary_function<std::shared_ptr<NetworkIdAddress>, uint16_t, bool>
{
    bool operator()(const first_argument_type left, second_argument_type right) const
    {
        if (left->networkId == right)
            return true; 

        return false;
    }
};

/**********************class TransmitConfig**********************/
class TransmitConfig
{
public:
    TransmitConfig();

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
    std::string tsFilesDir;
    std::list<std::shared_ptr<NetworkIdAddress>> netAddresses;    

    uint32_t   ftpInterval;
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
    NetworkRelationConfig();
    NetworkRelationConfig(const char *xmlFile);
    
    bool IsChildNetwork(uint16_t ancestor, uint16_t offspring);

public:
    //map<networkId, parentNetworkId>
    std::map<uint16_t, uint16_t> relations;
};

#endif