/*  history:
2015-07-06 Created by LiuHao.
*/
#include "Include/Foundation/SystemInclude.h"
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