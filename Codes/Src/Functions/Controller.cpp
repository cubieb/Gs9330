#include "SystemInclude.h"
#include "Common.h"

#include "Descriptor.h"
#include "Nit.h"
#include "XmlDataWrapper.h"
#include "Controller.h"

using namespace std;

void Func(const NitWrapper<Nit>& wrapper)
{
    Nit nit;
    wrapper.FillNit(nit);

    uchar_t buffer[1024];
    nit.MakeCodes(buffer, 1024);
}

void TestXmlDataRrapper()
{
    NitWrapper<Nit> *nitWrapper; 
    nitWrapper = new NitXmlWrapper<Nit>("../XmlFiles/Nit.xml", function<void(const NitWrapper<Nit>&)>(Func));

    Func(*nitWrapper);
}