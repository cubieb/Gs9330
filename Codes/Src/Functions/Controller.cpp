#include "SystemInclude.h"
#include <io.h>
#include "Common.h"
#include "Debug.h"

#include "Descriptor.h"
#include "Nit.h"
#include "Sdt.h"
#include "Bat.h"
#include "XmlDataWrapper.h"
#include "Ts.h"
#include "Gs9330Config.h"
#include "Controller.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

Controller::Controller()
{
    NetworkIpConfigWrapper networkIpConfigWrapper;
    networkIpConfigWrapper.ReadConfig(netConfig);

    XmlConfigWrapper xmlWrapper;
    xmlWrapper.ReadConfig(xmlConfig);
    
    for (auto iter: netConfig.entries)
    {
        std::map<uint16_t, std::shared_ptr<TsSnInfo>> tsSns;
        tsSns.insert(make_pair(0x0010, make_shared<TsSnInfo>(make_shared<Ts>(0x0010), InvalidSectionNumber)));
        tsSns.insert(make_pair(0x0011, make_shared<TsSnInfo>(make_shared<Ts>(0x0011), InvalidSectionNumber)));

        netTsSnInfors.insert(make_pair(iter.networkId, tsSns));
    }

#if defined(_DEBUG)
    /* PID, lastSectionSn => tsDebug */
    tsDebug.insert(make_pair(0x0010, InvalidSectionNumber));
    tsDebug.insert(make_pair(0x0011, InvalidSectionNumber));
#endif

    DataWrapper::DbInsertHandler handler(bind(&Controller::HandleDbInsert, this, _1, _2, _3));
    wrappers.push_back(make_shared<NitXmlWrapper<Nit>>(handler, xmlConfig.xmlDir.c_str()));
    wrappers.push_back(make_shared<SdtXmlWrapper<Sdt>>(handler, xmlConfig.xmlDir.c_str()));
    wrappers.push_back(make_shared<BatXmlWrapper<Bat>>(handler, xmlConfig.xmlDir.c_str()));
}

void Controller::Start()
{
    for (auto iter: wrappers)
    {
        iter->Start();
    }

    myThread = std::thread(bind(&Controller::ThreadMain, this));
}

void Controller::HandleDbInsert(uint16_t netId, shared_ptr<Section> section, uint16_t sectionSn)
{
    string xmlPath = xmlConfig.xmlDir + string("/") + string("NetWorkNode.xml");

    if (_access(xmlPath.c_str(), 0) != -1)
    {
        NetworkRelationConfigWrapper relationWrapper(xmlPath.c_str());    
        relationWrapper.ReadConfig(relationConfig);
        //remove(xmlPath.c_str());
    }

    for (auto tsSnInfors: netTsSnInfors)
    {
        if (!relationConfig.IsChildNetwork(tsSnInfors.first, netId))
        {
            continue;
        }

        lock_guard<mutex> lock(myMutext);
        auto tsSnInfor = tsSnInfors.second.find(section->GetPid());
        if (tsSnInfor->second->sn != sectionSn)
        {
            tsSnInfor->second->ts->Clear();
            tsSnInfor->second->sn = sectionSn;
        }
        tsSnInfor->second->ts->AddSection(section);

    }
}

void Controller::SendUdp(int socketFd)
{
    for (auto tsSnInfors: netTsSnInfors)
    {
        list<NetworkIpConfig::Entry>::iterator ipAddr;
        for (ipAddr = netConfig.entries.begin(); 
            ipAddr != netConfig.entries.end();
            ++ipAddr)
        {
            if (ipAddr->networkId = tsSnInfors.first)
                break;
        }
        assert(ipAddr != netConfig.entries.end());

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(ipAddr->tsDstPort);
        serverAddr.sin_addr.s_addr = ipAddr->tsDstIp;

        lock_guard<mutex> lock(myMutext);
        for(auto tsSnInfor: tsSnInfors.second)
        {
            if (tsSnInfor.second->ts->GetSectionNumber() == 0)
            {
                continue;
            }

            size_t size = tsSnInfor.second->ts->GetCodesSize();
            shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
            tsSnInfor.second->ts->MakeCodes(buffer.get(), size);
            int pktNumber = (size + 1459) / 1460;
            for (int i = 0; i < pktNumber; ++i)
            {
                int udpSize = min(size - 1460 * i, 1460);
                sendto(socketFd, (char*)buffer.get() + 1460 * i, (int)udpSize, 0, 
                       (SOCKADDR *)&serverAddr, 
                       sizeof(struct sockaddr_in));
            }

#if defined(_DEBUG)
            auto dbg = tsDebug.find(tsSnInfor.second->ts->GetPid());
            if (dbg->second != tsSnInfor.second->sn)
            {
                char file[MAX_PATH];
                sprintf(file, "D:/Temp/%03d%03d.ts",
                        tsSnInfor.second->ts->GetPid(),
                        tsSnInfor.second->sn);
    
                fstream tsFile(file, ios_base::out  | ios::binary);
                tsFile.write((char*)buffer.get(), size); 
                tsFile.close();

                dbg->second = tsSnInfor.second->sn;
            }
#endif
        }
    }
}

void Controller::ThreadMain()
{
    int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    while (true)
    {
        SendUdp(socketFd);
        Sleep(netConfig.tsInterval);
    }
    closesocket(socketFd);
}
