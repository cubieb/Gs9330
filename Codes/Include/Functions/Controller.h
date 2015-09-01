#ifndef _Controller_h_
#define _Controller_h_

class Nit;
class Sdt;
class Bat;
class Ts;
class DataWrapper;
struct Config;

#define InvalidSectionNumber -1

struct TsSnInfo
{
    std::shared_ptr<Ts> ts;

    /* Section Serial Number is used to identify if two .xml should packed into
       a group of TS packet.
     */
    uint16_t sn;

    TsSnInfo(std::shared_ptr<Ts> ts, uint16_t sn)
        : ts(ts), sn(sn)
    {}
};

class Controller
{
public:
    void Start();
    void HandleDbInsert(uint16_t netId, std::shared_ptr<Section> section, uint16_t sectionSn);

    static Controller& GetInstance()
    {
        static Controller instance;
        return instance;
    }

private:
    Controller();
    void SendUdp(int socketFd);
    void ThreadMain();
    std::thread myThread;
    std::mutex  myMutext;

    NetworkIpConfig       netConfig;
    XmlConfig             xmlConfig;
    NetworkRelationConfig relationConfig;
    /* NetwordId, PID => TsSnInfo */
    std::map<uint16_t, std::map<uint16_t, std::shared_ptr<TsSnInfo>>> netTsSnInfors;
    std::list<std::shared_ptr<DataWrapper>> wrappers;

#if defined(_DEBUG)
    /* PID, lastSectionSn => tsDebug */
    std::map<uint16_t, uint16_t> tsDebug;
#endif
};

#endif