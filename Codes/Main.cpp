/*  history:
2015-07-06 Created by LiuHao.
*/

#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"
#include "SystemError.h" 

#include "Descriptor.h"
#include "Controller.h"
#include "Nit.h"
#include "Sdt.h"
#include "Bat.h"
#include "Ts.h"
#include "Main.h"

using namespace std;

int main(int argc, char **argv) 
{
#if 1
    Controller& controller = Controller::GetInstance();
    controller.Start();
#else

#endif
    return 0;
}