#ifndef _Networks_h_
#define _Networks_h_

#include "Include/Configuration/NetworkCfgInterface.h"

/**********************class Receiver**********************/
class Receiver: public ReceiverInterface
{
public:
    Receiver(ReceiverId receiverId, TsId tsId, const struct sockaddr_in &dstAddr);
    ~Receiver();
    
    void Add(Pid from, Pid to);
    iterator Begin();
    virtual iterator End();
    struct sockaddr_in GetDstAddr() const;
    NodePtr GetMyHead();
    ReceiverId GetReceiverId() const;
    TsId GetTsId() const;   
    void Put(std::ostream& os) const;

private:
    ReceiverId receiverId;
    TsId tsId;
    struct sockaddr_in dstAddr;
    std::list<PidMap>  pidMaps;
};

/**********************class NetworkCfg**********************/
class NetworkCfg: public NetworkCfgInterface
{
public:
    NetworkCfg(NetId netId, const struct in_addr &srcAddr);
    ~NetworkCfg();

    void Add(ReceiverInterface *receiver);
    iterator Begin();
    iterator End();
    NodePtr GetMyHead();

    NetId GetNetId() const;    
    NetId GetParentNetId() const;
    NetId SetParentNetId(NetId netId);
    struct in_addr GetSrcAddr() const;

    void Put(std::ostream& os) const;

private:
    NetId netId;
    NetId parentNetId;
    struct in_addr srcAddr;
    std::list<ReceiverInterface *> receivers;
};

/**********************class NetworkCfgs**********************/
class NetworkCfgs: public NetworkCfgsInterface
{
public:
    NetworkCfgs();
    ~NetworkCfgs();

    void Add(NetworkCfgInterface *network);
    iterator Begin();
    void Delete(NetId netId);
    iterator End();
    iterator Find(NetId netId);

    // ContainerBase function, work with Iterator.
    NodePtr GetMyHead();

    bool IsChildNetwork(NetId ancestor, NetId offspring) const;

    void Put(std::ostream& os) const;

private:
    std::list<NetworkCfgInterface *> networks;
};

#endif