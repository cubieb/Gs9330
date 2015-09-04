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
    for (auto iter: tranmitConfig.netAddresses)
    {
        std::map<uint16_t, std::shared_ptr<TsSnInfo>> tsSns;
        tsSns.insert(make_pair(0x0010, make_shared<TsSnInfo>(make_shared<Ts>(0x0010), InvalidSerialNumber)));
        tsSns.insert(make_pair(0x0011, make_shared<TsSnInfo>(make_shared<Ts>(0x0011), InvalidSerialNumber)));

        netTsSnInfors.insert(make_pair(iter->networkId, tsSns));
    }

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
        relationConfig = make_shared<NetworkRelationConfig>(xmlPath.c_str());   
        //remove(xmlPath.c_str());
    }

    for (auto tsSnInfors: netTsSnInfors)
    {
        uint16_t networkId = tsSnInfors.first;
        if (!relationConfig->IsChildNetwork(netId, tsSnInfors.first))
        {
            continue;
        }

        lock_guard<mutex> lock(myMutext);
        auto tsSnInfor = tsSnInfors.second.find(section->GetPid());
        uint16_t& sn = tsSnInfor->second->sn;
        if (sn != sectionSn)
        {
            tsSnInfor->second->ts->Clear();    
            sn = sectionSn;
        }
        tsSnInfor->second->ts->AddSection(section);
        
#if defined(_DEBUG)
        bitset<256> tableIds;
        tableIds.set(section->GetTableId());
        size_t size = tsSnInfor->second->ts->GetCodesSize(tableIds);
        shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
        tsSnInfor->second->ts->MakeCodes(buffer.get(), size, tableIds);
        ostringstream file;
        file << "D:/Temp/NetId" <<  setfill('0') << setw(3) << (uint32_t)networkId 
            << ".Pid" <<  setfill('0') << setw(4) << hex << tsSnInfor->second->ts->GetPid() 
            << ".Sn" << setfill('0') << setw(3) << dec << sectionSn << ".ts";

        fstream tsFile(file.str(), ios_base::app  | ios::binary);
        tsFile.write((char*)buffer.get(), size); 
        tsFile.close();
#endif        
    }
}

void Controller::SendUdpToNetId(int socketFd, 
                                struct sockaddr_in& serverAddr,
                                std::bitset<256>& tableIds, 
                                std::map<uint16_t, std::shared_ptr<TsSnInfo>>& tsPids)
{
    lock_guard<mutex> lock(myMutext);

    /* Send upd packet for every TS(with specific PID) */
    for(auto tsSnInfor: tsPids)
    {
        uint16_t pid = tsSnInfor.first;
        size_t size = tsSnInfor.second->ts->GetCodesSize(tableIds);
        if (size == 0)
            continue;

        shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
        tsSnInfor.second->ts->MakeCodes(buffer.get(), size, tableIds);
        int pktNumber = (size + 1459) / 1460;
        for (int i = 0; i < pktNumber; ++i)
        {
            int udpSize = min(size - 1460 * i, 1460);
            sendto(socketFd, (char*)buffer.get() + 1460 * i, (int)udpSize, 0, 
                   (SOCKADDR *)&serverAddr, 
                   sizeof(struct sockaddr_in));
        }
    }
}

void Controller::SendUdp(int socketFd, bitset<256>& tableIds)
{
    for (auto tsSnInfors: netTsSnInfors)
    {
        uint16_t networkId = tsSnInfors.first;

        list<shared_ptr<NetworkIdAddress>>::iterator iter;
        for (iter = tranmitConfig.netAddresses.begin(); 
            iter != tranmitConfig.netAddresses.end();
            ++iter)
        {
            if ((*iter)->networkId == networkId)
                break;
        }
        assert(iter != tranmitConfig.netAddresses.end());
        NetworkIdAddress& ipAddr = *(*iter);

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(ipAddr.tsDstPort);
        serverAddr.sin_addr.s_addr = ipAddr.tsDstIp;

        SendUdpToNetId(socketFd, serverAddr, tableIds, tsSnInfors.second);        
    }
}

void Controller::ThreadMain()
{
    nitActualTimer = tranmitConfig.nitActualInterval;
    nitOtherTimer = tranmitConfig.nitOtherInterval;
    batTimer = tranmitConfig.batInterval;
    sdtActualTimer = tranmitConfig.sdtActualInterval;
    sdtOtherTimer = tranmitConfig.sdtOtherInterval;
    eit4ETimer = tranmitConfig.eit4EInterval;
    eit4FTimer = tranmitConfig.eit4FInterval;
    eit50to5FTimer = tranmitConfig.eit50Interval;
    eit60to6FTimer = tranmitConfig.eit60Interval;

    int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bitset<256> tableIds;
    while (true)
    {
        tableIds.reset();

        TheckTimer(nitActualTimer, tranmitConfig.nitActualInterval, tableIds, 0x40);
        TheckTimer(nitOtherTimer, tranmitConfig.nitOtherInterval, tableIds, 0x41);
        TheckTimer(batTimer, tranmitConfig.batInterval, tableIds, 0x4A);
        TheckTimer(sdtActualTimer, tranmitConfig.sdtActualInterval, tableIds, 0x42);
        TheckTimer(sdtOtherTimer, tranmitConfig.sdtOtherInterval, tableIds, 0x46);
        TheckTimer(eit4ETimer, tranmitConfig.eit4EInterval, tableIds, 0x4E);
        TheckTimer(eit4FTimer, tranmitConfig.eit4FInterval, tableIds, 0x4F);
        TheckTimer(eit50to5FTimer, tranmitConfig.eit50Interval, tableIds, 0x50);
        TheckTimer(eit60to6FTimer, tranmitConfig.eit60Interval, tableIds, 0x60);

        if (tableIds.any())
            SendUdp(socketFd, tableIds);

        Sleep(1000);
    }
    closesocket(socketFd);
}

void Controller::TheckTimer(uint32_t& cur, uint32_t orignal, bitset<256>& bits, uchar_t indexInBits)
{
    if (--cur == 0)
    {
        cur = orignal;
        bits.set(indexInBits);
    }
}
