#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Configuration */
#include "NetworksCfg.h"
using namespace std;

ReceiverInterface * CreateReceiverInterface(uint_t receiverId, const struct sockaddr_in &socketAddr)
{
    return new Receiver(receiverId, socketAddr);
}

NetworkCfgInterface * CreateNetworkCfgInterface(NetId netId)
{
    return new NetworkCfg(netId);
}

NetworkCfgsInterface * CreateNetworkCfgsInterface()
{
    return new NetworkCfgs;
}

/**********************class Receiver**********************/
/* public function */
Receiver::Receiver(uint_t receiverId, const struct sockaddr_in &socketAddr)
    : receiverId(receiverId), socketAddr(socketAddr)
{
    AllocProxy();
}

Receiver::~Receiver()
{
    FreeProxy();
}

void Receiver::Add(TsId tsId)
{
    tsIds.push_back(tsId);
}

Receiver::iterator Receiver::Begin()
{
    return iterator(this, NodePtr(tsIds.begin()));
}
 
void Receiver::Delete(TsId tsId)
{
    remove(tsIds.begin(), tsIds.end(), tsId);
}

Receiver::iterator Receiver::End()
{
    return iterator(this, NodePtr(tsIds.end()));
}

Receiver::iterator Receiver::Find(TsId tsId)
{
    list<TsId>::iterator iter;
    iter = find(tsIds.begin(), tsIds.end(), tsId);
    return iterator(this, iter);
}

Receiver::NodePtr Receiver::GetMyHead()
{
    return NodePtr(tsIds.end());
}

uint_t Receiver::GetReceiverId() const
{
    return receiverId;
}

struct sockaddr_in Receiver::GetSocketAddr() const
{
    return socketAddr;
}

void Receiver::Put(std::ostream& os) const
{
    os << "  ip = " << inet_ntoa(socketAddr.sin_addr) 
       << ", port = " << ntohs(socketAddr.sin_port) << endl;
    list<TsId>::const_iterator iter;
    for (iter = tsIds.begin(); iter != tsIds.end(); ++iter)
    {
        os << "    " << *iter << endl;
    }
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

void NetworkCfg::Delete(const struct sockaddr_in &socketAddr)
{
    list<ReceiverInterface *>::iterator iter;
    iter = find_if(receivers.begin(), receivers.end(), CompareReceiverSocketAddr(socketAddr));
    if (iter != receivers.end())
    {
        receivers.erase(iter);
        delete *iter;
    }
}

NetworkCfg::iterator NetworkCfg::End()
{
    return iterator(this, NodePtr(receivers.end()));
}

NetworkCfg::iterator NetworkCfg::Find(const struct sockaddr_in &socketAddr)
{
    list<ReceiverInterface *>::iterator iter;
    iter = find_if(receivers.begin(), receivers.end(), CompareReceiverSocketAddr(socketAddr));
    return iterator(this, iter);
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
