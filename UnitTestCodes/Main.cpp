/*  history:
2015-07-06 Created by LiuHao.
*/

#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"
#include "RouterError.h" 
#include "UnitTest.h"
#include "Main.h"

using namespace std;

int main()
{
    DebugFlag flag;
    flag.SetState("UnitTest::Crack", true);

    UnitTest::Crack();
    
    system("Pause");
	return 0;
}
