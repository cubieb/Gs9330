#ifndef _ControllerInterface_h_
#define _ControllerInterface_h_

#include "ace/Reactor.h"

class ControllerInterface: public ACE_Event_Handler
{
public:
    ControllerInterface() {};
    virtual ~ControllerInterface() {};

    virtual void Start(ACE_Reactor *reactor) = 0;
};

ControllerInterface * CreateControllerInterface();

#endif