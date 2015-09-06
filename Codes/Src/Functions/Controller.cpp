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
        std::map<uint16_t, std::shared_ptr<Ts>> pidTses;
        pidTses.insert(make_pair(0x0010, make_shared<Ts>(0x0010)));
        pidTses.insert(make_pair(0x0011, make_shared<Ts>(0x0011)));

        netPidTsInfors.insert(make_pair(iter->networkId, pidTses));
    }

    DataWrapper::DbInsertHandler handler(bind(&Controller::HandleDbInsert, this, _1, _2, _3));
    wrappers.push_back(make_shared<NitXmlWrapper<Nit>>(handler, xmlConfig.xmlDir.c_str()));
    wrappers.push_back(make_shared<SdtXmlWrapper<Sdt>>(handler, xmlConfig.xmlDir.c_str()));
    wrappers.push_back(make_shared<BatXmlWrapper<Bat>>(handler, xmlConfig.xmlDir.c_str()));

    relationConfig = make_shared<NetworkRelationConfig>(); 
}

void Controller::Start()
{
    for (auto iter: wrappers)
    {
        iter->Start();
    }

    myThread = std::thread(bind(&Controller::ThreadMain, this));
}

void Controller::HandleDbInsert(uint16_t netId, shared_ptr<Section> section, uint16_t netPidSn)
{
    string xmlPath = xmlConfig.xmlDir + string("/") + string("NetWorkNode.xml");

    uint16_t pid = section->GetPid();
    uint16_t oldSn = InvalidSerialNumber;
    auto netPidSnIter = netPidSnInfors.find(netId);
    if (netPidSnIter == netPidSnInfors.end())
    {
        std::map<uint16_t, uint16_t> pidSnInfor;
        pidSnInfor.insert(make_pair(pid, netPidSn));
        netPidSnInfors.insert(make_pair(netId, pidSnInfor));
    }
    else
    {
        auto pidSnIter = netPidSnIter->second.find(pid);
        if (pidSnIter == netPidSnIter->second.end())
            netPidSnIter->second.insert(make_pair(pid, netPidSn));
        else
            oldSn = pidSnIter->second;
    }

    if (_access(xmlPath.c_str(), 0) != -1)
    {
        relationConfig = make_shared<NetworkRelationConfig>(xmlPath.c_str());   
        //remove(xmlPath.c_str());  //delete "NetWorkNode.xml"
    }

    for (auto netPidTsIter: netPidTsInfors)
    {
        uint16_t networkId = netPidTsIter.first;

        if (!relationConfig->IsChildNetwork(netId, netPidTsIter.first))
        {
            continue;
        }

        lock_guard<mutex> lock(myMutext);
        auto pidTsIter = netPidTsIter.second.find(pid);
        
        if (oldSn != netPidSn)
        {
            pidTsIter->second->Clear(networkId);  
        }
        pidTsIter->second->AddSection(section);
        
#if defined(_DEBUG)
        bitset<256> tableIds;
        tableIds.set();
        size_t size = pidTsIter->second->GetCodesSize(tableIds);
        shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
        pidTsIter->second->MakeCodes(buffer.get(), size, tableIds);
        ostringstream file;
        file << "D:/Temp/NetId" <<  setfill('0') << setw(3) << (uint32_t)networkId 
            << ".Pid" <<  setfill('0') << setw(4) << hex << pidTsIter->second->GetPid() 
            << ".Sn" << setfill('0') << setw(3) << dec << netPidSn << ".ts";

        fstream tsFile(file.str(), ios_base::out  | ios::binary);
        tsFile.write((char*)buffer.get(), size); 
        tsFile.close();
#endif        
    }
}

void Controller::SendUdpToNetId(int socketFd, 
                                struct sockaddr_in& serverAddr,
                                std::bitset<256>& tableIds, 
                                std::map<uint16_t, std::shared_ptr<Ts>>& pidTsInfors)
{
    lock_guard<mutex> lock(myMutext);

#define UdpPayloadSize (188*7)
    /* Send upd packet for every TS(with specific PID) */
    for(auto pidTsIter: pidTsInfors)
    {
        uint16_t pid = pidTsIter.first;
        size_t size = pidTsIter.second->GetCodesSize(tableIds);
        if (size == 0)
            continue;

        shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
        pidTsIter.second->MakeCodes(buffer.get(), size, tableIds);

        int pktNumber = (size + UdpPayloadSize - 1) / UdpPayloadSize;
        for (int i = 0; i < pktNumber; ++i)
        {
            int udpSize = min(size - UdpPayloadSize * i, UdpPayloadSize);
            sendto(socketFd, (char*)buffer.get() + UdpPayloadSize * i, (int)udpSize, 0, 
                   (SOCKADDR *)&serverAddr, 
                   sizeof(struct sockaddr_in));
        }
    }
}

void Controller::SendUdp(int socketFd, bitset<256>& tableIds)
{
    for (auto netPidTsIter: netPidTsInfors)
    {
        uint16_t networkId = netPidTsIter.first;

        list<shared_ptr<NetworkIdAddress>>::iterator iter;
        iter = find_if(tranmitConfig.netAddresses.begin(), 
                       tranmitConfig.netAddresses.end(),
                       bind2nd(CmpNetworkIdAddressId(), networkId));
        if(iter == tranmitConfig.netAddresses.end())
        {
            /* can't find ip configuration, do nothing. */
            continue;
        }
        NetworkIdAddress& ipAddr = *(*iter);

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(ipAddr.dstPort);
        serverAddr.sin_addr.s_addr = inet_addr(ipAddr.dstIp.c_str());

        SendUdpToNetId(socketFd, serverAddr, tableIds, netPidTsIter.second);        
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
        //TheckTimer(eit4ETimer, tranmitConfig.eit4EInterval, tableIds, 0x4E);
        //TheckTimer(eit4FTimer, tranmitConfig.eit4FInterval, tableIds, 0x4F);
        //TheckTimer(eit50to5FTimer, tranmitConfig.eit50Interval, tableIds, 0x50);
        //TheckTimer(eit60to6FTimer, tranmitConfig.eit60Interval, tableIds, 0x60);

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
