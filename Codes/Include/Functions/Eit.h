#ifndef _Eit_h_
#define _Eit_h_

#include "Component.h"

class Eit: public Section
{
public:
    /* SI PID definition: <iso13818-1.pdf>, 5.1.3 Coding of PID and table_id fields */
    enum: uint16_t {Pid = 0x0012};
    Eit();
    ~Eit() {}
    
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);
    uchar_t GetTableId();
};

#endif