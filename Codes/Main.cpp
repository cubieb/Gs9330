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
#include "Ts.h"
#include "Main.h"

using namespace std;

int main(int argc, char **argv) 
{
#if 0
    Controller& controller = Controller::GetInstance();
    controller.Start();
#else
    shared_ptr<int> p1 = make_shared<int>(1);
    shared_ptr<int> p2;
    p2 = p1;

    cout << p1.use_count() << endl;
#endif
    return 0;
}