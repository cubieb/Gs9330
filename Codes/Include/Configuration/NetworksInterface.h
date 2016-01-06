#ifndef _NetworkInterface_h_
#define _NetworkInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/ContainerBase.h"

class ReceiverInterface: public ContainerBase
{
public:
    typedef std::list<TsId> Repository;
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
    
    typedef Iterator<TsId>::MyIter      iterator;
    typedef ConstIterator<TsId>::MyIter const_iterator;

    ReceiverInterface() {};
    virtual ~ReceiverInterface() {}

    virtual iterator Begin() = 0;
    virtual void Delete(NetId netId, Pid pid) = 0;
    virtual iterator End() = 0;
    virtual iterator Find(NetId netId, Pid pid) = 0;

    virtual struct sockaddr_in GetSocketAddr() = 0;
};

class NetworkInterface: public ContainerBase
{
public:
    typedef std::list<TsId> Repository;
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
    
    typedef Iterator<TsId>::MyIter      iterator;
    typedef ConstIterator<TsId>::MyIter const_iterator;

    NetworkInterface() {};
    virtual ~NetworkInterface() {}

    virtual NetId GetNetId() = 0;

    virtual iterator Begin() = 0;
    virtual void Delete(NetId netId, Pid pid) = 0;
    virtual iterator End() = 0;
    virtual iterator Find(NetId netId, Pid pid) = 0;
};

class NetworksInterface: public ContainerBase
{
public:    
    typedef std::list<NetworkInterface> Repository;
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
    
    typedef Iterator<NetworkInterface>::MyIter      iterator;
    typedef ConstIterator<NetworkInterface>::MyIter const_iterator;
    
    NetworksInterface() {};
    virtual ~NetworksInterface() {}    

    virtual iterator Begin() = 0;
    virtual void Delete(NetId netId, Pid pid) = 0;
    virtual iterator End() = 0;
    virtual iterator Find(NetId netId, Pid pid) = 0;
};

#endif