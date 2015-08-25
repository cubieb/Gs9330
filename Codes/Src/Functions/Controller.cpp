#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Descriptor.h"
#include "Nit.h"
#include "Sdt.h"
#include "XmlDataWrapper.h"
#include "Ts.h"
#include "Controller.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

Controller::Controller()
    : nitTsFile("D:/Temp/ActualAndOther.ts", ios_base::out  | ios::binary), 
      sdtTsFile("D:/Temp/Sdt.ts", ios_base::out  | ios::binary), 
      nitTs(new Ts), sdtTs(new Ts)
{
    function<void(const DataWrapper<Nit>&)> nitTrigger(bind(&Controller::NitTrigger, this, _1));

    DataWrapper<Nit> *nit;
    nit = new NitXmlWrapper<Nit>(nitTrigger, "../XmlFiles/Nit.101.Actual.xml");
    nitWrappers.push_back(shared_ptr<DataWrapper<Nit>>(nit));

    nit = new NitXmlWrapper<Nit>(nitTrigger, "../XmlFiles/Nit.101.Other.xml");
    nitWrappers.push_back(shared_ptr<DataWrapper<Nit>>(nit));

    nit = new NitXmlWrapper<Nit>(nitTrigger, "../XmlFiles/Nit.102.Other.xml");
    nitWrappers.push_back(shared_ptr<DataWrapper<Nit>>(nit));

    function<void(const DataWrapper<Sdt>&)> sdtTrigger(bind(&Controller::SdtTrigger, this, _1));

    DataWrapper<Sdt> *sdt;
    sdt = new SdtXmlWrapper<Sdt>(sdtTrigger, "../XmlFiles/SDT.xml");
    sdtWrappers.push_back(shared_ptr<DataWrapper<Sdt>>(sdt));
}

void Controller::Start() const
{
    for (auto iter: nitWrappers)
    {
        iter->Start();
    }

    for (auto iter: sdtWrappers)
    {
        iter->Start();
    }
}

void Controller::NitTrigger(const DataWrapper<Nit>& wrapper)
{
    Nit nit;
    wrapper.Fill(nit);
        
    size_t size = nitTs->GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    nitTs->MakeCodes(nit, buffer.get(), size);
    nitTsFile.write((char*)buffer.get(), size); 
}

void Controller::SdtTrigger(const DataWrapper<Sdt>& wrapper)
{
    Sdt sdt;
    wrapper.Fill(sdt);
        
    size_t size = sdtTs->GetCodesSize(sdt);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    sdtTs->MakeCodes(sdt, buffer.get(), size);
    sdtTsFile.write((char*)buffer.get(), size); 
}
