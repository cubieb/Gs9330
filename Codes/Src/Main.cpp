/*  history:
2015-07-06 Created by LiuHao.
*/
#include "Include/Foundation/SystemInclude.h"
#include "ace/OS_main.h"
#include "ace/Reactor.h"

/* Foundation */
#include "Include/Foundation/Type.h"

/* ControllerInterface */
#include "Include/Controller/ControllerInterface.h"

/* SiTableWrapper */
#include "Include/SiTableWrapper/SiTableXmlWrapperInterface.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TsPacketInterface.h"

using namespace std;

void Test()
{
    ACE_Reactor *reactor = ACE_Reactor::instance();
    auto_ptr<ControllerInterface> controller(CreateControllerInterface(reactor));

    list<TsId> tsIds;
    tsIds.push_back(1);
    tsIds.push_back(2);

    reactor->run_reactor_event_loop();
}

int main(int argc, char **argv) 
{
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    Test();

    return 0;
}