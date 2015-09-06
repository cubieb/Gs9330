#ifndef _Controller_h_
#define _Controller_h_

class Nit;
class Sdt;
class Bat;
class Ts;
class DataWrapper;
struct Config;

#define InvalidSerialNumber -1

class Controller
{
public:
    void Start();
    void HandleDbInsert(uint16_t netId, std::shared_ptr<Section> section, uint16_t netPidSn);

    static Controller& GetInstance()
    {
        static Controller instance;
        return instance;
    }

private:
    Controller();
    void SendUdpToNetId(int socketFd, 
                        struct sockaddr_in& serverAddr,
                        std::bitset<256>& tableIds, 
                        std::map<uint16_t, std::shared_ptr<Ts>>& pidTsInfors);
    void SendUdp(int socketFd, std::bitset<256>& tableIds);
    void ThreadMain();
    void TheckTimer(uint32_t& cur, uint32_t orignal, std::bitset<256>& bits, uchar_t indexInBits);

    std::thread myThread;
    std::mutex  myMutext;

    TransmitConfig        tranmitConfig;
    XmlConfig             xmlConfig;
    std::shared_ptr<NetworkRelationConfig> relationConfig;

    /* NetwordId, PID => Section Serial Number */
    std::map<uint16_t, std::map<uint16_t, uint16_t>> netPidSnInfors;

    /* NetwordId, PID => TsSnInfo */
    std::map<uint16_t, std::map<uint16_t, std::shared_ptr<Ts>>> netPidTsInfors;
    std::list<std::shared_ptr<DataWrapper>> wrappers;

    uint32_t   nitActualTimer;
    uint32_t   nitOtherTimer;
    uint32_t   batTimer;
    uint32_t   sdtActualTimer;
    uint32_t   sdtOtherTimer;
    uint32_t   eit4ETimer;
    uint32_t   eit4FTimer;
    uint32_t   eit50to5FTimer;
    uint32_t   eit60to6FTimer;
};

#endif