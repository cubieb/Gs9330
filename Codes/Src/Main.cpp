/*  history:
2015-07-06 Created by LiuHao.
*/
#include "Include/Foundation/SystemInclude.h"
#include "ace/OS_main.h"
#include "ace/Reactor.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* ControllerInterface */
#include "Include/Controller/ControllerInterface.h"
using namespace std;

int main(int argc, char **argv)
{
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    
    DebugFlag debugFlag;
    debugFlag.SetState("Controller::handle_close", true);

    ACE_Reactor *reactor = ACE_Reactor::instance();
    auto_ptr<ControllerInterface> controller(CreateControllerInterface(reactor));

    reactor->run_reactor_event_loop();
    return 0;
}