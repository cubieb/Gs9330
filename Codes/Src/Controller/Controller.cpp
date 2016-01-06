#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TsPacketInterface.h"

/* Controller */
#include "ace/OS_main.h"
#include "Controller.h"
using namespace std;

ControllerInterface * CreateControllerInterface(ACE_Reactor *reactor)
{
    return new Controller(reactor);
}

/**********************class Controller**********************/
/* public function */
Controller::Controller(ACE_Reactor *reactor)
{
    this->reactor (reactor);

    tableNameToPid.insert(make_pair("nit", NitPid));
    tableNameToPid.insert(make_pair("bat", BatPid));
    tableNameToPid.insert(make_pair("sdt", SdtPid));
    tableNameToPid.insert(make_pair("eit", EitPid));

    tableIdToPid.insert(make_pair(NitActualTableId, NitPid));
    tableIdToPid.insert(make_pair(NitOtherTableId, NitPid));
    tableIdToPid.insert(make_pair(SdtActualTableId, SdtPid));
    tableIdToPid.insert(make_pair(SdtOtherTableId, SdtPid));
    tableIdToPid.insert(make_pair(BatTableId, BatPid));
    tableIdToPid.insert(make_pair(EitActualPfTableId, EitPid));
    tableIdToPid.insert(make_pair(EitOtherPfTableId, EitPid));
    tableIdToPid.insert(make_pair(EitActualSchTableId, EitPid));
    tableIdToPid.insert(make_pair(EitOtherSchTableId, EitPid));

    tsPackets = CreateTsPacketsInterface();

    AddMonitoredDir("D:\\Project.VS\\Gs9330\\InputFiles\\XmlFiles");
    AddMonitoredDir("D:\\Project.VS\\Gs9330\\InputFiles\\TsFiles");

    ReadFile("D:\\Project.VS\\Gs9330\\InputFiles\\XmlFiles\\201_004_nit__2015-12-15-10-34-01.xml");
}

Controller::~Controller()
{
    delete tsPackets;
}

int Controller::handle_signal(int signum, siginfo_t *, ucontext_t *)
{
    DWORD waitStatus;
    waitStatus = WaitForMultipleObjects(2, &handles[0], FALSE, INFINITE); 
    if (waitStatus >= WAIT_OBJECT_0 && waitStatus < WAIT_OBJECT_0 + handles.size())
    {
        ::FindNextChangeNotification(handles[waitStatus - WAIT_OBJECT_0]);
    }

    return 0;
}

int Controller::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
    dbgstrm << "Controller removed from Reactor."  << endl;
    ::FindCloseChangeNotification(handle);

    vector<ACE_HANDLE>::iterator end = remove(handles.begin(), handles.end(), handle);
    handles.erase(end, handles.end());

    return 0;
}

int Controller::handle_timeout(const ACE_Time_Value &currentTime,
                               const void *act)
{
    uint32_t timerArg = *(uint32_t *)act;
    NetId netId = timerArg >> 16;
    TableId tableId = timerArg & 0xff;
    Pid pid = tableIdToPid.find(tableId)->second;

    TsPacketsInterface::iterator iter = tsPackets->Find(netId, pid);
    assert(iter != tsPackets->End());

    return 0;
}

/* private function */
void Controller::AnalyzeFileName(const char *path, NetId &netId, Pid &pid)
{    
   char drive[3];
   char dir[PATH_MAX];
   char fname[FILENAME_MAX];
   char ext[FILENAME_MAX];

    _splitpath(path, drive, dir, fname, ext); 

    char buffer[FILENAME_MAX];
    char *start, *end;
    size_t size;

    /* Network Id */
    start = fname;
    end = strstr(fname, "_");
    size = end - start;
    memcpy(buffer, start, size);
    buffer[size] = '\0';
    netId = (NetId)strtol(buffer, nullptr, 0);

    /* version */
    start = end + 1;
    end = strstr(start, "_");

    /* nit|bat|sdt|eit */
    start = end + 1;
    end = strstr(start, "_");
    memcpy(buffer, start, size);
    buffer[size] = '\0';
    map<string, Pid>::iterator iter = tableNameToPid.find(buffer);
    assert(iter != tableNameToPid.end());
    pid = iter->second;    
}

void Controller::AddMonitoredDir(char *directory)
{
    int flags = FILE_NOTIFY_CHANGE_FILE_NAME;

    ACE_HANDLE handle = ACE_TEXT_FindFirstChangeNotification(directory,  // pointer to name of directory to watch
        FALSE, // flag for monitoring directory or directory tree
        flags); // filter conditions to watch for        
    assert(handle != ACE_INVALID_HANDLE);

    handles.push_back(handle);

    int ret;
    ret = this->reactor()->register_handler(this, handle);
    assert(ret == 0);
}

/* name rule:  netId_version_[nit|bat|sdt|eit]__year-month-day-hour-minuts-seconds.xml
example: 201_004_nit__2015-12-15-10-34-01.xml 
*/
void Controller::ReadFile(char *path)
{
    NetId   netId;
    Pid     pid;
    TableId tableId = 0;
    AnalyzeFileName(path, netId, pid);

    /* Add SiSable to TsPacket */
    TsPacketInterface *tsPacket;
    TsPacketsInterface::iterator iter = tsPackets->Find(netId, pid);
    if (iter == tsPackets->End())
    {        
        tsPacket = CreateTsPacketInterface(netId, pid);
        tsPackets->Add(tsPacket);
    }
    else
    {
        tsPacket = *iter;
    }
    tsPacket->AddXmlTable(path, tableId);

    /* schedule timer for {NetId, TableId} */
    uint32_t timerArg = (netId << 16) | tableId;
    list<uint32_t>::iterator timerIter;
    timerIter = find(timerArgs.begin(), timerArgs.end(), timerArg);
    if (timerIter == timerArgs.end())
    {
        timerArgs.push_back(timerArg);
        timerIter = find(timerArgs.begin(), timerArgs.end(), timerArg);        

        ACE_Time_Value delay(2);   // Two seconds
        ACE_Reactor *reactor = this->reactor();    
        reactor->schedule_timer(this, &(*timerIter), ACE_Time_Value::zero, delay);
    }    
}