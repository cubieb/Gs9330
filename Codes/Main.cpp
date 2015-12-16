/*  history:
2015-07-06 Created by LiuHao.
*/
#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"
#include "SystemError.h" 

#include "Descriptor.h"
#include "Gs9330Config.h"
#include "Controller.h"
#include "Nit.h"
#include "Sdt.h"
#include "Bat.h"
#include "Ts.h"
#include "DirMonitor.h"

using namespace std;

#include <regex>

int main(int argc, char **argv) 
{
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

#if 1
    Controller& controller = Controller::GetInstance();
    controller.Start();
#else

#endif

    while(true)
    {
        SleepEx(2000, true);  //must be SleepEx(), Sleep will not work
    }
    return 0;
}