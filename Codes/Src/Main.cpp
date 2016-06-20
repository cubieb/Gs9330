/*  history:
2015-07-06 Created by LiuHao.
*/
#include "Include/Foundation/SystemInclude.h"

#pragma warning(push)
#pragma warning(disable:702)   //disable warning caused by ACE library.
#pragma warning(disable:4251)  //disable warning caused by ACE library.
#pragma warning(disable:4996)  //disable warning caused by ACE library.
#include "ace/OS.h"
#include "ace/OS_main.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* ControllerInterface */
#include "Include/Controller/ControllerInterface.h"
#include "resource.h"
#include "Version.h"
using namespace std;

int main(int argc, char **argv)
{   
    /* to keep there is only one instance is running */
    HANDLE mutex;
    mutex = CreateMutex(NULL, TRUE, TEXT("EpgSender")); 
    if ((mutex != nullptr) && (GetLastError() == ERROR_ALREADY_EXISTS)) 
    {
        /* One another instance is running! */
        return 1;
    }

    HWND hwnd=GetConsoleWindow();
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)));
    
    cout << "interal version: " << ExeVersion << endl;
    char *dir = (argc == 1 ? "ReceiveXml" : argv[1]);
    if ((ACE_OS::access(dir, F_OK)) != 0)
    {
        errstrm << dir << " do not exist!" << endl;
        system("pause");
        return 1;
    }

    ControllerInterface &controller = ControllerInterface::GetInstance();
    if (!controller.Start(ACE_Reactor::instance(), dir))
    {
        errstrm << "controller.Start() failed. " << endl;
        system("pause");
        return 1;
    }

    ACE_Reactor::instance()->run_reactor_event_loop();

    if (mutex != nullptr)
    {
        ReleaseMutex(mutex);
        CloseHandle(mutex);
    }

    return 0;
}

#pragma warning(pop) 