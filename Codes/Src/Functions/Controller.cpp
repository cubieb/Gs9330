#include "SystemInclude.h"
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
    ConfigDataWrapper wrapper;
    wrapper.ReadConfig(config);

    function<void(Section&, uint16_t)> handler(bind(&Controller::HandleDbInsert, this, _1, _2));

    auto nitTs = make_shared<Ts>();
    tsInfors.insert(make_pair(Nit::ClassId, TsRuntimeInfo(InvalidSectionNumber, "D:/Temp/Nit.ts", nitTs)));
    wrappers.push_back(make_shared<NitXmlWrapper<Nit>>(handler, config.xmlDir.c_str()));

    auto sdtTs = make_shared<Ts>();
    tsInfors.insert(make_pair(Sdt::ClassId, TsRuntimeInfo(InvalidSectionNumber, "D:/Temp/Sdt.ts", sdtTs)));
    wrappers.push_back(make_shared<SdtXmlWrapper<Sdt>>(handler, config.xmlDir.c_str()));

    auto batTs = make_shared<Ts>();
    tsInfors.insert(make_pair(Bat::ClassId, TsRuntimeInfo(InvalidSectionNumber, "D:/Temp/Bat.ts", batTs)));
    wrappers.push_back(make_shared<BatXmlWrapper<Bat>>(handler, config.xmlDir.c_str()));
}

void Controller::Start()
{
    for (auto iter: wrappers)
    {
        iter->Start();
    }

    myThread = std::thread(bind(&Controller::ThreadMain, this));
}

void Controller::HandleDbInsert(Section& section, uint16_t sectionSn)
{
    uint16_t classId= section.GetClassId();
    auto iter = tsInfors.find(classId);
    auto tsFileName = iter->second.tsOutPutFileNmae;
    auto ts = iter->second.tsInstance;

#if !defined(SendModeFile)
    {
        lock_guard<mutex> lock(myMutext);
        if (sectionSn != iter->second.sectionSn && iter->second.tsBinary.size() != 0)
        {        
            iter->second.tsBinary.clear();
        }
    }
#endif

    size_t size = ts->GetCodesSize(section);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    ts->MakeCodes(section, buffer.get(), size);    

#if defined(_DEBUG)
    /* write data to Ts file */
    ios_base::openmode mode;
    if (sectionSn == iter->second.sectionSn)
    {
        mode = ios_base::app  | ios::binary;
    }
    else
    {
        mode = ios_base::out  | ios::binary;
    }
    fstream tsFile(tsFileName, mode);
    tsFile.write((char*)buffer.get(), size); 
    tsFile.close();
#endif

#if !defined(SendModeFile)
    {
        lock_guard<mutex> lock(myMutext);
        iter->second.tsBinary.push_back(make_pair(buffer, size)); 
    }
#endif
    iter->second.sectionSn = sectionSn;
}

void Controller::SendUdp(int socketFd)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config.tsDstPort);
    serverAddr.sin_addr.s_addr = config.tsDstIp;

    for (auto iter: tsInfors)
    {
#if defined(SendModeFile)
        auto tsFileName = iter.second.tsOutPutFileNmae;
        fstream tsFile(tsFileName, ios_base::in  | ios::binary);

        tsFile.seekg (0, tsFile.end);
        int length = (int)tsFile.tellg();
        tsFile.seekg (0, tsFile.beg);

        shared_ptr<uchar_t> buffer(new uchar_t[length], UcharDeleter());
        tsFile.read((char*)buffer.get(), length); 
        tsFile.close();
#else
        int length = 0;
        shared_ptr<uchar_t> buffer;
        {
            lock_guard<mutex> lock(myMutext);
            for (auto buffer: iter.second.tsBinary)
            {
                length = length + buffer.second;
            }
            buffer.reset(new uchar_t[length], UcharDeleter());
            uchar_t *ptr = buffer.get();
            for (auto buffer: iter.second.tsBinary)
            {
                memcpy(ptr, buffer.first.get(), buffer.second);
                ptr = ptr +  buffer.second;
            }
        }
#endif
        int pktNumber = (length + 1459) / 1460;
        for (int i = 0; i < pktNumber; ++i)
        {
            int udpSize = min(length - 1460 * i, 1460);
            sendto(socketFd, (char*)buffer.get() + 1460 * i, (int)udpSize, 0, 
                  (SOCKADDR *)&serverAddr, 
                  sizeof(struct sockaddr_in));
        }
    }
}

void Controller::ThreadMain()
{
    int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    while (true)
    {
        SendUdp(socketFd);
        Sleep(config.tsInterval);
    }
    closesocket(socketFd);
}
