#ifndef _Controller_h_
#define _Controller_h_

/* Configuration */
#include "Include/Configuration/TimerCfgInterface.h"
#include "Include/Configuration/DirCfgInterface.h"
#include "Include/Configuration/NetworkCfgInterface.h"

/* Controller */
#include "Include/Controller/ControllerInterface.h"
#include "TimerRepository.h"

struct FileSummary
{
    FileSummary(const std::string &fileName, TableId tableId, const std::list<SiTableKey> &keys)
        : fileName(fileName), tableId(tableId), keys(keys)
    {}

    FileSummary(const FileSummary &value)
        : fileName(value.fileName), tableId(value.tableId), keys(value.keys)
    {}

    FileSummary(FileSummary &&value)
    {
        fileName = move(value.fileName);
        tableId = value.tableId;
        keys = move(value.keys);
    }

    std::string        fileName;
    TableId            tableId;
    std::list<SiTableKey> keys;
};

class CompareSummaryFileName: public std::unary_function<FileSummary, bool>
{
public:
    CompareSummaryFileName(const char *fileName)
        : fileName(fileName)
    {}

    result_type operator()(const argument_type &summary)
    {
        return (result_type)(summary.fileName == fileName);
    }    

    result_type operator()(const argument_type *summary)
    {
        return this->operator()(*summary);
    }

private:
    std::string fileName;
};

/**********************class Controller**********************/
class Controller: public ControllerInterface
{
public:
    Controller();
    ~Controller();

    int handle_signal(int signum, siginfo_t *, ucontext_t *);
    int handle_timeout(const ACE_Time_Value &currentTime, const void *act);
    void Start(ACE_Reactor *reactor);

private:
    void AddMonitoredDir(const char *directory);
    
    /* name rule:  netId_version_[nit|bat|sdt|eit]_index_year-month-day-hour-minuts-seconds.xml
    example: 201_004_nit_1_2015-12-15-10-34-01.xml 
    */
    void AnalyzeFileName(const char *path, NetId &netId, Pid &pid, std::string &type);
    void AddSiTable(const char *path);
    void DelSiTable(const char *path);
    void ReadDir(const char *dir);

    void SendUdp(NetworkCfgInterface *network, TransportPacketInterface *tsPacket, TableId tableId);
    void ScheduleTimer(NetId netId, TableId tableId);

private:
    /* const data member */
    std::map<TableId, Pid>     tableIdToPid;
    std::map<std::string, Pid> tableNameToPid;

    /* configuration */
    DirCfgInterface *dirCfg;
    NetworkCfgsInterface *networkCfgs;
    TimerCfgInterface *timerCfg;

    /* runtime information */
    TransportPacketsInterface *tsPackets;    //modified by AddSiTable(), tsPackets->Add(tsPacket)
    TimerRepository *timerRepository; //modified by AddSiTable(), timerRepository->Add(timerId, timerArg);
    std::list<FileSummary>  fileSummaries; //modified by AddSiTable(), fileSummaries.push_back(fileSummary);
    ACE_HANDLE dirHandle;      //monitored dir handle, modified by AddMonitoredDir()
};

#endif