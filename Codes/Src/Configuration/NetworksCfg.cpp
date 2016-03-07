#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Configuration */
#include "NetworksCfg.h"
using namespace std;

ReceiverInterface * ReceiverInterface::CreateInstance(TsId tsId, 
                                                      const struct sockaddr_in &dstAddr, 
                                                      const struct in_addr &srcAddr,
                                                      Pid eitPid)
{
    return new Receiver(tsId, dstAddr, srcAddr, eitPid);
}

NetworkCfgInterface * NetworkCfgInterface::CreateInstance(NetId netId)
{
    return new NetworkCfg(netId);
}

NetworkCfgsInterface * NetworkCfgsInterface::CreateInstance()
{
    return new NetworkCfgs;
}

/**********************class Receiver**********************/
/* public function */
Receiver::Receiver(TsId tsId, const struct sockaddr_in &dstAddr, const struct in_addr &srcAddr, Pid eitPid)
    : tsId(tsId), dstAddr(dstAddr), srcAddr(srcAddr), eitPid(eitPid)
{
}

Receiver::~Receiver()
{
}

Pid Receiver::GetEitPid() const
{
    return eitPid;
}

struct sockaddr_in Receiver::GetDstAddr() const
{
    return dstAddr;
}

struct in_addr Receiver::GetSrcAddr() const
{
    return srcAddr;
}

TsId Receiver::GetTsId() const
{
    return tsId;
};

void Receiver::Put(std::ostream& os) const
{
    os << "  ip = " << inet_ntoa(dstAddr.sin_addr) 
       << ", port = " << ntohs(dstAddr.sin_port) 
       << ", tsId = " << tsId << endl;
}

/**********************class NetworkCfg**********************/
/* public function */
NetworkCfg::NetworkCfg(NetId netId)
    : netId(netId), parentNetId(0)
{
    AllocProxy();
}

NetworkCfg::~NetworkCfg()
{
    FreeProxy();
}

void NetworkCfg::Add(ReceiverInterface *receiver)
{
    receivers.push_back(receiver);
}

NetworkCfg::iterator NetworkCfg::Begin()
{
    return iterator(this, NodePtr(receivers.begin()));
}

NetworkCfg::iterator NetworkCfg::End()
{
    return iterator(this, NodePtr(receivers.end()));
}

NetworkCfg::NodePtr NetworkCfg::GetMyHead()
{
    return NodePtr(receivers.end());
}

NetId NetworkCfg::GetNetId() const
{
    return netId;
}

NetId NetworkCfg::GetParentNetId() const
{
    return parentNetId;
}

NetId NetworkCfg::SetParentNetId(NetId netId)
{
    return parentNetId = netId;
}

void NetworkCfg::Put(std::ostream& os) const
{
    os << "network id = " << netId << ", parent id = " << parentNetId << endl;
    list<ReceiverInterface *>::const_iterator iter;
    for (iter = receivers.begin(); iter != receivers.end(); ++iter)
    {
        os << **iter;
    }
}

/**********************class NetworkCfgs**********************/
/* public function */
NetworkCfgs::NetworkCfgs()
{
    AllocProxy();
}

NetworkCfgs::~NetworkCfgs()
{
    FreeProxy();
}

void NetworkCfgs::Add(NetworkCfgInterface *network)
{
    networks.push_back(network);
}

NetworkCfgs::iterator NetworkCfgs::Begin()
{
    return iterator(this, NodePtr(networks.begin()));
}

void NetworkCfgs::Delete(NetId netId)
{
    list<NetworkCfgInterface *>::iterator iter;
    iter = find_if(networks.begin(), networks.end(), CompareNetworkCfgId(netId));
    if (iter != networks.end())
    {
        networks.erase(iter);
        delete *iter;
    }
}

NetworkCfgs::iterator NetworkCfgs::End()
{
    return iterator(this, NodePtr(networks.end()));
}

NetworkCfgs::iterator NetworkCfgs::Find(NetId netId)
{
    list<NetworkCfgInterface *>::iterator iter;
    iter = find_if(networks.begin(), networks.end(), CompareNetworkCfgId(netId));
    return iterator(this, iter);
}

// ContainerBase function, work with Iterator.
NetworkCfgs::NodePtr NetworkCfgs::GetMyHead()
{
    return NodePtr(networks.end());
}

bool NetworkCfgs::IsChildNetwork(NetId ancestor, NetId offspring) const
{
    if (ancestor == offspring)
        return true;

    list<NetworkCfgInterface *>::const_iterator iter;
    for (iter = find_if(networks.begin(), networks.end(), CompareNetworkCfgId(offspring));
         iter != networks.end();
         iter = find_if(networks.begin(), networks.end(), CompareNetworkCfgId((*iter)->GetParentNetId())))
    {
        if ((*iter)->GetNetId() == ancestor)
        {
            return true;
        }
    }

    return false;
}

void NetworkCfgs::Put(std::ostream& os) const
{
    list<NetworkCfgInterface *>::const_iterator iter;
    for (iter = networks.begin(); iter != networks.end(); ++iter)
    {
        os << **iter;
    }
}
