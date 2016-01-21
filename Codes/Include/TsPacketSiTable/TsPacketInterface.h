#ifndef _TsPacketInterface_h_
#define _TsPacketInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/ContainerBase.h"

class SiTableInterface;

/**********************class TsPacketInterface**********************/
class TsPacketInterface
{
public:
    TsPacketInterface() {};
    virtual ~TsPacketInterface() {};

    virtual void AddSiTable(SiTableInterface *siTable) = 0;
    virtual void DelSiTable(TableId tableId, uint16_t key) = 0;

    virtual SiTableInterface * FindSiTable(TableId tableId, uint16_t key) = 0;
    virtual size_t GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const = 0;
    virtual NetId  GetNetId() const = 0;
    virtual Pid    GetPid() const = 0;
    virtual size_t MakeCodes(uint_t ccId, TableId tableId, std::list<TsId>& tsIds, 
                             uchar_t *buffer, size_t bufferSize) = 0;
};
TsPacketInterface * CreateTsPacketInterface(NetId netId, Pid pid);

class CompareTsPacketNetIdAndPid: public std::unary_function<TsPacketInterface, bool>
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

/**********************class TsPacketsInterface**********************/
class TsPacketsInterface: public ContainerBase
{
public:
    typedef std::list<TsPacketInterface*> Repository;
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
    
    typedef Iterator<TsPacketsInterface>::MyIter      iterator;
    typedef ConstIterator<TsPacketsInterface>::MyIter const_iterator;

    TsPacketsInterface() {};
    virtual ~TsPacketsInterface() {};

    virtual void Add(TsPacketInterface *tsPacket) = 0;

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
};
TsPacketsInterface * CreateTsPacketsInterface();

#endif