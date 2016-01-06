#ifndef _Controller_h_
#define _Controller_h_

/* Controller */
#include "Include/Controller/ControllerInterface.h"

/**********************class Controller**********************/
class Controller: public ControllerInterface
{
public:
    Controller(ACE_Reactor *reactor);
    ~Controller();

    int handle_signal(int signum, siginfo_t *, ucontext_t *);
    int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
    int handle_timeout(const ACE_Time_Value &currentTime, const void *act);

private:
    void AddMonitoredDir(char *directory);
    void AnalyzeFileName(const char *path, NetId &netId, Pid &pid);
    void ReadFile(char *path);

private:
    std::vector<ACE_HANDLE> handles;      //monitored dir handle
    std::map<std::string, Pid> tableNameToPid;
    std::map<TableId, Pid>     tableIdToPid;
    std::list<uint32_t> timerArgs;        //(NetId << 16) | (TableId)
    TsPacketsInterface *tsPackets;
};

#endif