#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Descriptor.h"
#include "Nit.h"
#include "XmlDataWrapper.h"
#include "Ts.h"
#include "Controller.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

Controller::Controller()
{
    string name("../XmlFiles/Nit.xml");

    function<void(const NitWrapper<Nit>&)> trigger(bind(&Controller::Trigger, this, _1));
    nitWrapper.reset(new NitXmlWrapper<Nit>(trigger, name));
}

void Controller::Start() const
{
    nitWrapper->Start();
}

struct TsIdPara
{
    uint16_t tsId;
    uint32_t freq;

};

void Controller::Trigger(const NitWrapper<Nit>& wrapper) const
{
#if 0
    Nit nit;
    wrapper.FillNit(nit);

    Ts ts;
    size_t size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());

    size_t ret = ts.MakeCodes(nit, buffer.get(), size);
    assert(ret == size);
#else
    TsIdPara tsid[] = 
    { 
        {1,  0x03790000}, {51, 0x04190000}, {52, 0x04270000}, {53, 0x04350000}, {54, 0x04430000}, 
        {55, 0x04510000}, {56, 0x04590000}, {57, 0x04740000}, {58, 0x04820000}, {59, 0x04980000}, 
        {60, 0x05060000}, {61, 0x05140000}, {62, 0x02270000}
    };
    
    Ts ts;
    fstream file("D:/Temp/ActualAndOther.ts", ios_base::out  | ios::binary);

    Nit nit;
    
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(0xc);
    char *netName = "辽宁北方广电网络";
    nit.AddDescriptor(0x40, (uchar_t*)netName, strlen(netName)); 

    for (auto iter: tsid)
    {
        nit.AddTs(iter.tsId, 0x1000);
        nit.AddTsDescriptor0x44(iter.tsId, 0x1000, iter.freq, 2, 3, 0x68750, 0);
    }
    
    size_t size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> bufferActual(new uchar_t[size], UcharDeleter());
    ts.MakeCodes(nit, bufferActual.get(), size);
    file.write((char*)bufferActual.get(), size); 
    
    nit.SetTableId(0x41);
    size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> bufferOther(new uchar_t[size], UcharDeleter());
    ts.MakeCodes(nit, bufferOther.get(), size);
    file.write((char*)bufferOther.get(), size); 
#endif
}

