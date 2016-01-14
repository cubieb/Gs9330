#ifndef _TimerCfg_h_
#define _TimerCfg_h_

#include "Include/Configuration/TimerCfgInterface.h"

class TimerCfg: public TimerCfgInterface
{
public:
    TimerCfg();
    ~TimerCfg();

    time_t GetInterval(TableId tableId) const;
    void SetInterval(TableId tableId, time_t sec);

private:
    std::map<TableId, time_t> timerCfg;
};

#endif