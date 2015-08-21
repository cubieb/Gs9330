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
    : file("D:/Temp/ActualAndOther.ts", ios_base::out  | ios::binary), ts(new Ts)
{
    function<void(const NitWrapper<Nit>&)> trigger(bind(&Controller::Trigger, this, _1));

    NitWrapper<Nit> *ptr;
    ptr = new NitXmlWrapper<Nit>(trigger, "../XmlFiles/Nit.101.Actual.xml");
    nitWrappers.push_back(shared_ptr<NitWrapper<Nit>>(ptr));

    ptr = new NitXmlWrapper<Nit>(trigger, "../XmlFiles/Nit.101.Other.xml");
    nitWrappers.push_back(shared_ptr<NitWrapper<Nit>>(ptr));

    ptr = new NitXmlWrapper<Nit>(trigger, "../XmlFiles/Nit.102.Other.xml");
    nitWrappers.push_back(shared_ptr<NitWrapper<Nit>>(ptr));
}

void Controller::Start() const
{
    for (auto iter: nitWrappers)
    {
        iter->Start();
    }
}

void Controller::Trigger(const NitWrapper<Nit>& wrapper)
{
    Nit nit;
    wrapper.FillNit(nit);
        
    size_t size = ts->GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    ts->MakeCodes(nit, buffer.get(), size);
    file.write((char*)buffer.get(), size); 
}

