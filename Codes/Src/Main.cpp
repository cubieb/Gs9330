/*  history:
2015-07-06 Created by LiuHao.
*/
#include "Include/Foundation/SystemInclude.h"

#pragma warning(push)
#pragma warning(disable:702)   //disable warning caused by ACE library.
#pragma warning(disable:4251)  //disable warning caused by ACE library.
#pragma warning(disable:4996)  //disable warning caused by ACE library.
#include "ace/OS_main.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* ControllerInterface */
#include "Include/Controller/ControllerInterface.h"
using namespace std;

int main(int argc, char **argv)
{    
    ControllerInterface &controller = ControllerInterface::GetInstance();
    controller.Start(ACE_Reactor::instance());

    ACE_Reactor::instance()->run_reactor_event_loop();

    return 0;
}

#pragma warning(pop) 