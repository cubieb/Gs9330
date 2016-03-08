#ifndef _NetworkCfgInterface_h_
#define _NetworkCfgInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/ContainerBase.h"

/**********************class ReceiverInterface**********************/
class ReceiverInterface: public ContainerBase
{
public:
    typedef std::pair<Pid, Pid> PidMap;
    typedef std::list<PidMap> Repository;
    class NodePtr
    {
    public:
        typedef Repository::iterator InerIter;
        NodePtr(): myIter()
        {}
        NodePtr(const InerIter& iter): myIter(iter)
        {}

        operator void*() const
        {
            return myIter._Ptr;
        }

        InerIter myIter;
    };

    typedef Repository::value_type value_type;
    typedef Repository::size_type size_type;
    typedef Repository::difference_type difference_type;
    typedef Repository::pointer pointer;
    typedef Repository::const_pointer const_pointer;
    typedef Repository::reference reference;
    typedef Repository::const_reference const_reference;
    
    typedef Iterator<ReceiverInterface>::MyIter      iterator;
    typedef ConstIterator<ReceiverInterface>::MyIter const_iterator;

    ReceiverInterface() {};
    virtual ~ReceiverInterface() {}    

    virtual void Add(Pid from, Pid to) = 0;
    // ContainerBase function. construct proxy from _Alnod
    void AllocProxy()
    {
        myProxy = new ContainerProxy;
        myProxy->myContainer = this;
    }

    virtual iterator Begin() = 0;
    virtual iterator End() = 0;

    // ContainerBase function, destroy proxy.
    void FreeProxy()
    {
        OrphanAll();
        delete myProxy;
        myProxy = nullptr;
    }
    virtual struct sockaddr_in GetDstAddr() const = 0;

    // ContainerBase function, work with Iterator.
    virtual NodePtr GetMyHead() = 0;

    // ContainerBase function.
    static NodePtr GetNextNodePtr(NodePtr ptr)
    {   // return reference to successor pointer in node
        ++ptr.myIter;
        return ptr;
    }

    virtual TsId GetTsId() const = 0;

    // ContainerBase function.
    static reference GetValue(NodePtr ptr)
    {
        return ((reference)*ptr.myIter);
    }

    /* the following function is provided just for debug */
    virtual void Put(std::ostream& os) const = 0;

    static ReceiverInterface * CreateInstance(TsId tsId, const struct sockaddr_in &dstAddr);
};

inline std::ostream& operator << (std::ostream& os, const ReceiverInterface& value) 
{ 
    value.Put(os); 
    return os; 
}

/**********************class NetworkInterface**********************/
class NetworkCfgInterface: public ContainerBase
{
public:
    typedef std::list<ReceiverInterface *> Repository;
    class NodePtr
    {
    public:
        typedef Repository::iterator InerIter;
        NodePtr(): myIter()
        {}
        NodePtr(const InerIter& iter): myIter(iter)
        {}

        operator void*() const
        {
            return myIter._Ptr;
        }

        InerIter myIter;
    };

    typedef Repository::value_type value_type;
    typedef Repository::size_type size_type;
    typedef Repository::difference_type difference_type;
    typedef Repository::pointer pointer;
    typedef Repository::const_pointer const_pointer;
    typedef Repository::reference reference;
    typedef Repository::const_reference const_reference;
    
    typedef Iterator<NetworkCfgInterface>::MyIter      iterator;
    typedef ConstIterator<NetworkCfgInterface>::MyIter const_iterator;

    NetworkCfgInterface() {};
    virtual ~NetworkCfgInterface() {}

    virtual void Add(ReceiverInterface *receiver) = 0;
    // ContainerBase function. construct proxy from _Alnod
    void AllocProxy()
    {
        myProxy = new ContainerProxy;
        myProxy->myContainer = this;
    }

    virtual iterator Begin() = 0;
    //virtual void Delete(const struct sockaddr_in &socketAddr) = 0;
    virtual iterator End() = 0;
    //virtual iterator Find(const struct sockaddr_in &socketAddr) = 0;
    
    // ContainerBase function, destroy proxy.
    void FreeProxy()
    {
        OrphanAll();
        delete myProxy;
        myProxy = nullptr;
    }

    // ContainerBase function, work with Iterator.
    virtual NodePtr GetMyHead() = 0;    
    virtual NetId GetNetId() const = 0;
    virtual NetId GetParentNetId() const = 0;
    virtual NetId SetParentNetId(NetId netId) = 0;
    virtual struct in_addr GetSrcAddr() const = 0;

    //static function
    // ContainerBase function.
    static NodePtr GetNextNodePtr(NodePtr ptr)
    {   // return reference to successor pointer in node
        ++ptr.myIter;
        return ptr;
    }

    // ContainerBase function.
    static reference GetValue(NodePtr ptr)
    {
        return ((reference)*ptr.myIter);
    }

    /* the following function is provided just for debug */
    virtual void Put(std::ostream& os) const = 0;

    static NetworkCfgInterface * CreateInstance(NetId netId, const struct in_addr &srcAddr);
};

inline std::ostream& operator << (std::ostream& os, const NetworkCfgInterface& value) 
{ 
    value.Put(os); 
    return os; 
}

class CompareNetworkCfgId: public std::unary_function<NetworkCfgInterface, bool>
{
public:
    CompareNetworkCfgId(NetId netId)
        : netId(netId)
    {}

    result_type operator()(const argument_type &network)
    {
        return (result_type)(network.GetNetId() == netId);
    }    

    result_type operator()(const argument_type *network)
    {
        return this->operator()(*network);
    }

private:
    NetId netId;
};

/**********************class NetworksInterface**********************/
/* Example:
    NetworksInterface *networks = CreateNetworksInterface();
    for (uint_t i = 0; i < 5; ++i)
    {
        NetworkInterface *network = CreateNetworkInterface((NetId)i);
        networks->Add(network);
    }

    NetworksInterface::iterator iter;
    for (iter = networks->Begin(); iter != networks->End(); ++iter)
    {
        cout << (*iter)->GetNetId() << endl;
    }
*/
class NetworkCfgsInterface: public ContainerBase
{
public:    
    typedef std::list<NetworkCfgInterface *> Repository;
    class NodePtr
    {
    public:
        typedef Repository::iterator InerIter;
        NodePtr(): myIter()
        {}
        NodePtr(const InerIter& iter): myIter(iter)
        {}

        operator void*() const
        {
            return myIter._Ptr;
        }

        InerIter myIter;
    };

    typedef Repository::value_type value_type;
    typedef Repository::size_type size_type;
    typedef Repository::difference_type difference_type;
    typedef Repository::pointer pointer;
    typedef Repository::const_pointer const_pointer;
    typedef Repository::reference reference;
    typedef Repository::const_reference const_reference;
    
    typedef Iterator<NetworkCfgsInterface>::MyIter      iterator;
    typedef ConstIterator<NetworkCfgsInterface>::MyIter const_iterator;
    
    NetworkCfgsInterface() {};
    virtual ~NetworkCfgsInterface() {}    

    virtual void Add(NetworkCfgInterface *network) = 0;
    // ContainerBase function. construct proxy from _Alnod
    void AllocProxy()
    {
        myProxy = new ContainerProxy;
        myProxy->myContainer = this;
    }

    virtual iterator Begin() = 0;
    virtual void Delete(NetId netId) = 0;
    virtual iterator End() = 0;
    virtual iterator Find(NetId netId) = 0;

    // ContainerBase function, destroy proxy.
    void FreeProxy()
    {
        OrphanAll();
        delete myProxy;
        myProxy = nullptr;
    }

    // ContainerBase function, work with Iterator.
    virtual NodePtr GetMyHead() = 0;   

    //static function
    // ContainerBase function.
    static NodePtr GetNextNodePtr(NodePtr ptr)
    {   // return reference to successor pointer in node
        ++ptr.myIter;
        return ptr;
    }

    // ContainerBase function.
    static reference GetValue(NodePtr ptr)
    {
        return ((reference)*ptr.myIter);
    }   

    virtual bool IsChildNetwork(NetId ancestor, NetId offspring) const = 0;

    /* the following function is provided just for debug */
    virtual void Put(std::ostream& os) const = 0;

    static NetworkCfgsInterface * CreateInstance();
};

inline std::ostream& operator << (std::ostream& os, const NetworkCfgsInterface& value) 
{ 
    value.Put(os); 
    return os; 
}

#endif