#ifndef _ControllerInterface_h_
#define _ControllerInterface_h_

#include "ace/Reactor.h"

class ControllerInterface: public ACE_Event_Handler
{
public:
    ControllerInterface() {};
    virtual ~ControllerInterface() {};
};

ControllerInterface * CreateControllerInterface(ACE_Reactor *reactor);

#endif