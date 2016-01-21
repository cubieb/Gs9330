#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Controller */
#include "TimerRepository.h"
using namespace std;

/**********************class TimerRepository**********************/
/* public function */
TimerRepository::TimerRepository()
{
    AllocProxy();
}

TimerRepository::~TimerRepository()
{
    Repository::iterator iter;
    for (iter = timers.begin(); iter != timers.end(); ++iter)
    {
        delete iter->second;
    }

    FreeProxy();
}

void TimerRepository::Add(TimerId timerId, TimerArg *timerInfo)
{
    timers.insert(make_pair(timerId, timerInfo));
}

TimerRepository::iterator TimerRepository::Begin()
{
    return iterator(this, NodePtr(timers.begin()));
}

void TimerRepository::Delete(TimerId timerId)
{
    Repository::iterator iter = timers.find(timerId);
    if (iter != timers.end())
        timers.erase(iter);
}

TimerRepository::iterator TimerRepository::End()
{
    return iterator(this, NodePtr(timers.end()));
}

TimerRepository::iterator TimerRepository::Find(TimerId timerId)
{
    return iterator(this, NodePtr(timers.find(timerId)));
}

TimerRepository::iterator TimerRepository::Find(TimerArg &timerInfo)
{
    Repository::iterator iter;
    for (iter = timers.begin(); iter != timers.end(); ++iter)
    {
        if (iter->second->netId == timerInfo.netId
            && iter->second->tableId == timerInfo.tableId)
        {
            return iterator(this, NodePtr(iter));
        }
    }
    return iterator(this, NodePtr(timers.end()));
}

TimerRepository::NodePtr TimerRepository::GetMyHead()
{
    return NodePtr(timers.end());
}