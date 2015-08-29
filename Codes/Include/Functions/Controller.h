#ifndef _Controller_h_
#define _Controller_h_

class Nit;
class Sdt;
class Bat;
class Ts;
class DataWrapper;
struct Config;

#define InvalidSectionNumber -1

//#define SendModeFile

struct TsRuntimeInfo
{
    /* Section Serial Numbers is used to identify if two .xml should packed into
       a single TS packet.
     */
    uint16_t sectionSn;

    /* when debug, we packet ts into output .ts files. */
    std::string tsOutPutFileNmae;
    std::shared_ptr<Ts> tsInstance;    
#if !defined(SendModeFile)
    std::list<std::pair<std::shared_ptr<uchar_t>, size_t>> tsBinary; /* catch for ready ts binary */
#endif

    TsRuntimeInfo(uint16_t sectionSn, std::string tsOutPutFileNmae,  std::shared_ptr<Ts> tsInstance)
        : sectionSn(sectionSn), 
          tsOutPutFileNmae(tsOutPutFileNmae),
          tsInstance(tsInstance) 
    { }

#if defined(SendModeFile)
    TsRuntimeInfo(const TsRuntimeInfo& right)
        : sectionSn(right.sectionSn), 
          tsOutPutFileNmae(right.tsOutPutFileNmae),
          tsInstance(right.tsInstance)
    { }
#else
    TsRuntimeInfo(const TsRuntimeInfo& right)
        : sectionSn(right.sectionSn), 
          tsOutPutFileNmae(right.tsOutPutFileNmae),
          tsInstance(right.tsInstance),
          tsBinary(right.tsBinary)
    { }
#endif
};

class Controller
{
public:
    typedef uint16_t SectionClassId;
    void Start();
    void HandleDbInsert(Section& section, uint16_t sectionSn);

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

    Config config;
    std::map<SectionClassId, TsRuntimeInfo> tsInfors;
    std::list<std::shared_ptr<DataWrapper>> wrappers;
};

#endif