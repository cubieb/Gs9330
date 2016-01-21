#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Configuration */
#include "TimerCfg.h"
using namespace std;

TimerCfgInterface * CreateTimerCfgInterface()
{
    return new TimerCfg;
}

/**********************class TimerCfg**********************/
/* public function */
TimerCfg::TimerCfg()
{
}

TimerCfg::~TimerCfg()
{
}

time_t TimerCfg::GetInterval(TableId tableId) const
{
    map<TableId, time_t>::const_iterator iter;
    iter = timerCfg.find(tableId);
    assert(iter != timerCfg.end());
    return iter->second;
}

void TimerCfg::SetInterval(TableId tableId, time_t sec)
{
    map<TableId, time_t>::iterator iter;
    iter = timerCfg.find(tableId);
    if (iter == timerCfg.end())
    {
        timerCfg.insert(make_pair(tableId, sec));
    }
    else
    {
        iter->second = sec;
    }
}