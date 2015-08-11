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

void Controller::Trigger(const NitWrapper<Nit>& wrapper) const
{
    Nit nit;
    wrapper.FillNit(nit);

    Ts ts;
    size_t size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());

    size_t ret = ts.MakeCodes(nit, buffer.get(), size);
    assert(ret == size);
}

