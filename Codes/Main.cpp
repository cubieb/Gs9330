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
    Nit nit;
    
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(0xc);
    char *name = "Royal cable";
    nit.AddDescriptor(0x40, (uchar_t*)name, strlen(name)); 

    shared_ptr<uchar_t> buffer;

    std::cout << nit;

    Ts ts;

    size_t size = ts.GetCodesSize(nit);
    buffer.reset(new uchar_t[size], UcharDeleter());
    size_t ret = ts.MakeCodes(nit, buffer.get(), size);

    fstream file("D:/Temp/Temp.ts", ios_base::out  | ios::binary);
    file.write((char*)buffer.get(), size); 
   
    return 0;
}