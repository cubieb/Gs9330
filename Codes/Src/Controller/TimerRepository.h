#ifndef _TimerRepository_h_
#define _TimerRepository_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/ContainerBase.h"

typedef int TimerId;
struct TimerArg
{
    TimerArg(NetId netId, TableId tableId)
        : netId(netId), tableId(tableId)
    {}

    NetId   netId;
    TableId tableId;
};

/**********************class TimerRepository**********************/
/* Example:
    timerRepository = new TimerRepository();
    timerRepository->Add(1, TimerInformation(1, 1));
    timerRepository->Add(2, TimerInformation(2, 2));
    for (TimerRepository::iterator iter = timerRepository->Begin(); iter != timerRepository->End(); ++iter)
    {
        cout << iter->first << ", {" << iter->second.netId << ", " << iter->second.tableId << "}" << endl;
    }
*/

class TimerRepository: public ContainerBase
{
public:
    typedef std::map<TimerId, TimerArg*> Repository;
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
    
    typedef Iterator<TimerRepository>::MyIter      iterator;
    typedef ConstIterator<TimerRepository>::MyIter const_iterator;

    TimerRepository();
    virtual ~TimerRepository();

    void Add(TimerId timerId, TimerArg *timerInfo);
    // ContainerBase function. construct proxy from _Alnod
    void AllocProxy()
    {
        myProxy = new ContainerProxy;
        myProxy->myContainer = this;
    } 

    iterator Begin();
    void Delete(TimerId timerId);
    iterator End();
    iterator Find(TimerId timerId);
    iterator Find(TimerArg &timerInfo);

    // ContainerBase function, destroy proxy.
    void FreeProxy()
    {
        OrphanAll();
        delete myProxy;
        myProxy = nullptr;
    }

    // ContainerBase function, work with Iterator.
    NodePtr GetMyHead();

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
    void Put(std::ostream& os) const;

private:
    Repository timers;
};

inline std::ostream& operator << (std::ostream& os, const TimerRepository& value) 
{ 
    value.Put(os); 
    return os; 
}



#endif

