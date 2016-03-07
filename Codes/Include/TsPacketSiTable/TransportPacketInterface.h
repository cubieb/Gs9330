#ifndef _TransportPacketInterface_h_
#define _TransportPacketInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/ContainerBase.h"

class SiTableInterface;

/**********************class TransportPacketInterface**********************/
//iso13818-1, 2.4.3.2 Transport Stream packet layer
class TransportPacketInterface
{
public:
    TransportPacketInterface() {};
    virtual ~TransportPacketInterface() {};

    virtual void AddSiTable(SiTableInterface *siTable) = 0;
    virtual void DelSiTable(TableId tableId, SiTableKey key) = 0;

    virtual SiTableInterface * FindSiTable(TableId tableId, SiTableKey key) = 0;
    virtual size_t GetCodesSize(TableId tableId, TsId tsId) const = 0;
    virtual NetId  GetNetId() const = 0;
    virtual Pid    GetPid() const = 0;
    virtual size_t MakeCodes(CcId ccId, TableId tableId, TsId tsId, 
                             uchar_t *buffer, size_t bufferSize) = 0;
    virtual void MapPid(uchar_t *buffer, size_t bufferSize, Pid from, Pid to) const = 0;
    virtual void RefreshCatch() = 0;

    static TransportPacketInterface * CreateInstance(NetId netId, Pid pid);
};

class CompareTsPacketNetIdAndPid: public std::unary_function<TransportPacketInterface, bool>
{
public:
    CompareTsPacketNetIdAndPid(NetId netId, Pid pid)
        : netId(netId), pid(pid)
    {}

    result_type operator()(const argument_type &tsPacket)
    {
        return (result_type)(tsPacket.GetNetId() == netId && tsPacket.GetPid() == pid);
    }    

    result_type operator()(const argument_type *tsPacket)
    {
        return this->operator()(*tsPacket);
    }

private:
    NetId netId;
    Pid   pid;
};

/**********************class TransportPacketsInterface**********************/
class TransportPacketsInterface: public ContainerBase
{
public:
    typedef std::list<TransportPacketInterface*> Repository;
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
    
    typedef Iterator<TransportPacketsInterface>::MyIter      iterator;
    typedef ConstIterator<TransportPacketsInterface>::MyIter const_iterator;

    TransportPacketsInterface() {};
    virtual ~TransportPacketsInterface() {};

    virtual void Add(TransportPacketInterface *tsPacket) = 0;

    // ContainerBase function. construct proxy from _Alnod
    void AllocProxy()
    {
        myProxy = new ContainerProxy;
        myProxy->myContainer = this;
    }  
    virtual iterator Begin() = 0;
    virtual void Delete(NetId netId, Pid pid) = 0;
    virtual iterator End() = 0;
    virtual iterator Find(NetId netId, Pid pid) = 0;

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

    static TransportPacketsInterface * CreateInstance();
};

#endif