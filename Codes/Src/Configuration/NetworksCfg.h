#ifndef _Networks_h_
#define _Networks_h_

#include "Include/Configuration/NetworkCfgInterface.h"

/**********************class Receiver**********************/
class Receiver: public ReceiverInterface
{
public:
    Receiver(TsId tsId, const struct sockaddr_in &socketAddr);
    ~Receiver();
    
    struct sockaddr_in GetSocketAddr() const;
    TsId GetTsId() const;   
    void Put(std::ostream& os) const;

private:
    TsId tsId;
    struct sockaddr_in socketAddr;
};

/**********************class NetworkCfg**********************/
class NetworkCfg: public NetworkCfgInterface
{
public:
    NetworkCfg(NetId netId);
    ~NetworkCfg();

    void Add(ReceiverInterface *receiver);
    iterator Begin();
    void Delete(const struct sockaddr_in &socketAddr);
    iterator End();
    iterator Find(const struct sockaddr_in &socketAddr);
    NodePtr GetMyHead();

    NetId GetNetId() const;    
    NetId GetParentNetId() const;
    NetId SetParentNetId(NetId netId);

    void Put(std::ostream& os) const;

private:
    NetId netId;
    NetId parentNetId;
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