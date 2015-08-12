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
    //function<void(const NitWrapper<Nit>&)> trigger(bind(&TsCase::Trigger, this, _1));
    //NitXmlWrapper<Nit> nitWrapper(trigger, "../XmlFiles/Nit.xml"); 
    //nitWrapper.FillNit(nit);
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(1);
    char *name = "Chengdu Broadcast Television Network";
    nit.AddDescriptor(0x40, (uchar_t*)name, strlen(name)); 
    nit.AddTransportStream(5, 5);
 
    cout << nit;

    Ts ts;
    size_t size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    fstream file("D:/Temp/Temp.ts", ios_base::out  | ios::binary);
    size_t ret = ts.MakeCodes(nit, buffer.get(), size);
    file.write((char*)buffer.get(), size); 
    
    return 0;
}