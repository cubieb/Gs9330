#ifndef _TimerCfgInterface_h_
#define _TimerCfgInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"

class TimerCfgInterface
{
public:
    TimerCfgInterface() {};
    virtual ~TimerCfgInterface() {};

    virtual time_t GetInterval(TableId tableId) const = 0;
    virtual void SetInterval(TableId tableId, time_t sec) = 0;

    static TimerCfgInterface * CreateInstance();
};

#endif