#include "Include/Foundation/SystemInclude.h"
#include <regex>
#include "ace/OS.h"
#include "ace/Singleton.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* ConfigurationWrapper */
#include "Include/ConfigurationWrapper/TimerCfgWrapperInterface.h"
#include "Include/ConfigurationWrapper/DirCfgWrapperInterface.h"
#include "Include/ConfigurationWrapper/NetworkCfgWrapperInterface.h"
#include "Include/ConfigurationWrapper/NetworkRelationWrapperInterface.h"

/* SiTableWrapper */
#include "Include/SiTableWrapper/SiTableXmlWrapperInterface.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TsPacketInterface.h"

/* Controller */
#include "TimerRepository.h"
#include "Controller.h"
using namespace std;

/**********************SiTableXmlWrapperRepository**********************/
static SiTableXmlWrapperAutoRegisterSuite<TsPacketInterface> batWrapper
    (string("bat"), new BatXmlWrapper<TsPacketInterface, BatTableInterface>);

static SiTableXmlWrapperAutoRegisterSuite<TsPacketInterface> eitWrapper
    (string("eit"), new EitXmlWrapper<TsPacketInterface, EitTableInterface>);

static SiTableXmlWrapperAutoRegisterSuite<TsPacketInterface> nitWrapper
    (string("nit"), new NitXmlWrapper<TsPacketInterface, NitTableInterface>);

static SiTableXmlWrapperAutoRegisterSuite<TsPacketInterface> sdtWrapper
    (string("sdt"), new SdtXmlWrapper<TsPacketInterface, SdtTableInterface>);

ControllerInterface &ControllerInterface::GetInstance()
{
    typedef ACE_Singleton<Controller, ACE_Recursive_Thread_Mutex> TheController;

    return *TheController::instance();
}


/**********************class Controller**********************/
/* public function */
Controller::Controller()
    : tsPackets(nullptr), timerRepository(nullptr)
{
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
    
    /* dir configuration */
    dirCfg = CreateDirCfgInterface();
    DirCfgWrapperInterface<DirCfgInterface> dirCfgWrapper;
    dirCfgWrapper.Select(*dirCfg, "Gs9330SoapClient.xml");
    
    /* networks configuration */
    string receiverCfgPath = string(dirCfg->GetCfgDir()) + string("\\receiver.xml");
    networkCfgs = CreateNetworkCfgsInterface();
    NetworkCfgWrapperInterface<NetworkCfgsInterface> networkCfgWrapper;
    networkCfgWrapper.Select(*networkCfgs, receiverCfgPath.c_str());

    /* timer configuration */
    string senderCfgPath = string(dirCfg->GetCfgDir()) + string("\\sender.xml");
    timerCfg = CreateTimerCfgInterface();
    TimerCfgWrapperInterface<TimerCfgInterface> timerCfgWrapper;
    timerCfgWrapper.Select(*timerCfg, senderCfgPath.c_str());    
}

Controller::~Controller()
{
    ACE_Reactor *reactor = this->reactor();
    
    /* stop monitoring dir 
    * remove_handler() may put event in Reactor's event queue, and the the event will cause a
    * invoke to ACE_Event_Handler::handle_close(). 
    * now we call remove_handler() in destruct function, that ACE_Event_Handler::handle_close()
    * will occur at sometime this Controller is invalid.  In order to avoid invalid calling to
    * handle_close(), we set the ACE_Event_Handler::DONT_CALL bit.
    */
    ACE_Reactor_Mask mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
    reactor->remove_handler(dirHandle, mask);
    ::FindCloseChangeNotification(dirHandle);
    dirHandle = ACE_INVALID_HANDLE;

    /* clear file summary */
    for_each(fileSummaries.begin(), fileSummaries.end(), ScalarDeleter());

    /* cancle timers */
    TimerRepository::iterator iter;
    for (iter = timerRepository->Begin(); iter != timerRepository->End(); ++iter)
    {
        reactor->cancel_timer(iter->first);
    }
    delete timerRepository;
    
    delete tsPackets;

    /* free configuration */
    delete networkCfgs;
    delete dirCfg;
    delete timerCfg;
}

int Controller::handle_signal(int signum, siginfo_t *, ucontext_t *)
{
    DWORD waitStatus;
    waitStatus = WaitForMultipleObjects(1, &dirHandle, FALSE, INFINITE); 
    if (waitStatus != WAIT_OBJECT_0)
    {
        return 0;
    }

    string ok = string(dirCfg->GetXmlDir()) + string("\\ok");
	while((ACE_OS::access(ok.c_str(), F_OK)) != 0 )
    {
        Sleep(10);
    }
    
    ReadDir(dirCfg->GetXmlDir());

    ::FindNextChangeNotification(dirHandle);
    return 0;
}

int Controller::handle_timeout(const ACE_Time_Value &currentTime,
                               const void *act)
{
    string pause = string(dirCfg->GetXmlDir()) + string("\\pause");
    if (ACE_OS::access(pause.c_str(), F_OK) == 0)
    {
        return 0;
    }

    TimerArg &timerArg = *(TimerArg *)act;
    NetId netId = timerArg.netId;
    TableId tableId = timerArg.tableId;
    Pid pid = tableIdToPid.find(tableId)->second;

    TsPacketsInterface::iterator tsPacketIter = tsPackets->Find(netId, pid);
    assert(tsPacketIter != tsPackets->End());

    NetworkCfgsInterface::iterator networkIter;
    for (networkIter = networkCfgs->Begin(); networkIter != networkCfgs->End(); ++networkIter)
    {
        NetworkCfgInterface *network = *networkIter;
        if (!networkCfgs->IsChildNetwork(netId, network->GetNetId()))
        {
            continue;
        }
        
        SendUdp(network, *tsPacketIter, tableId);
    }   
    
    return 0;
}

void Controller::Start(ACE_Reactor *reactor)
{
    this->reactor (reactor);

    /* this->tsPackets */
    tsPackets = CreateTsPacketsInterface();    

    /* Timer Repository(timer runtimer information) */
    timerRepository = new TimerRepository();
    
    ReadDir(dirCfg->GetXmlDir());
    AddMonitoredDir(dirCfg->GetXmlDir());
    //AddMonitoredDir(dirCfg->GetTsDir()); 
}

/* private function */
void Controller::AnalyzeFileName(const char *path, NetId &netId, Pid &pid, string &type)
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
    size = end - start;
    memcpy(buffer, start, size);
    buffer[size] = '\0';
    map<string, Pid>::iterator iter = tableNameToPid.find(buffer);
    assert(iter != tableNameToPid.end());
    pid = iter->second;
    type = buffer;
}

void Controller::AddMonitoredDir(const char *directory)
{
    int flags = FILE_NOTIFY_CHANGE_FILE_NAME;

    dirHandle = ACE_TEXT_FindFirstChangeNotification(directory,  
        FALSE, // flag for monitoring directory or directory tree
        flags); // filter conditions to watch for        
    assert(dirHandle != ACE_INVALID_HANDLE);
    
    int ret;
    ret = this->reactor()->register_handler(this, dirHandle);
    assert(ret == 0);
}

void Controller::AddSiTable(const char *path)
{
    NetId   netId;
    Pid     pid;
    string  type;
    
    AnalyzeFileName(path, netId, pid, type);    
    
    /* network relation */
    NetworkRelationWrapperInterface<NetworkCfgsInterface> netRelationWrapper;
    string relationXmlPath = string(dirCfg->GetXmlDir()) + string("\\NetWorkNode.xml");
    netRelationWrapper.Select(*networkCfgs, relationXmlPath.c_str());
        
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
    TableId tableId;
    list<uint16_t> keys;    
    SiTableXmlWrapperRepository<TsPacketInterface> &siTableWrapperRepository =
        SiTableXmlWrapperRepository<TsPacketInterface>::GetInstance();
    SiTableXmlWrapperInterface<TsPacketInterface> &siTableWrapper = 
        siTableWrapperRepository.GetWrapperInstance(type);
    siTableWrapper.Select(*tsPacket, path, tableId, keys);
    
    /* save {file name, table id, key list} relation ship */
    FileSummary *fileSummary = new FileSummary;
    fileSummary->fileName = path;
    fileSummary->tableId = tableId;
    fileSummary->keys = keys;
    fileSummaries.push_back(fileSummary);
        
    /* schedule timer for {NetId, TableId} */
    ScheduleTimer(netId, tableId);
    if (tableId == EitActualSchTableId)
    {
        ScheduleTimer(netId, EitActualPfTableId);
    }
    else if (tableId == EitOtherSchTableId)
    {
        ScheduleTimer(netId, EitOtherPfTableId);
    }
}

void Controller::DelSiTable(const char *path)
{
    NetId   netId;
    Pid     pid;
    string  type;
    cout << "Removing " << path << endl;
    AnalyzeFileName(path, netId, pid, type);   

    TsPacketsInterface::iterator iter = tsPackets->Find(netId, pid);
    assert(iter != tsPackets->End());
    TsPacketInterface *tsPacket = *iter;

    std::list<FileSummary *>::iterator summaryIter;
    for (summaryIter = fileSummaries.begin(); summaryIter != fileSummaries.end(); ++summaryIter)
    {
        if ((*summaryIter)->fileName.compare(path) != 0)
        {
            continue;
        }

        list<uint16_t>::iterator keyIter;
        for (keyIter = (*summaryIter)->keys.begin(); keyIter != (*summaryIter)->keys.end(); ++keyIter)
        {
            tsPacket->DelSiTable((*summaryIter)->tableId, *keyIter);
        }
    }

    for_each(fileSummaries.begin(), fileSummaries.end(), DeleteSummaryFileName(path));
    fileSummaries.remove(nullptr);
}

void Controller::ReadDir(const char *dir)
{    
    list<string> newPathes, oldPathes;
    
#if 0
    string path = string(dir) + string("\\*.xml");
    _finddata_t fileInfo;  
    long handle = _findfirst(path.c_str(), &fileInfo); 
    if (handle != -1) 
    {        
        do
        {
            dbgstrm << fileInfo.name << endl; 
            if (regex_match(fileInfo.name, regex(".*(nit|bat|sdt|eit).*\\.xml")))
            {
                newPathes.push_back(string(dir) + string("\\") + string(fileInfo.name));
            } 
        } while (_findnext(handle, &fileInfo) == 0); 
    }
#else    
	ACE_DIR *aceDir = ACE_OS::opendir(dir);
	assert(aceDir != nullptr);
	while (ACE_DIRENT *entry = ACE_OS::readdir(aceDir))
	{
		if (regex_match(entry->d_name, regex(".*(nit|bat|sdt|eit).*\\.xml")))
        {
            newPathes.push_back(string(dir) + string("\\") + string(entry->d_name));
        }
	}
	ACE_OS::closedir(aceDir);
#endif


    list<FileSummary *>::iterator summeryIter;
    for (summeryIter = fileSummaries.begin(); summeryIter != fileSummaries.end(); ++summeryIter)
    {
        oldPathes.push_back((*summeryIter)->fileName);
    }

    list<string>::iterator newIter, oldIter;
    for (newIter = newPathes.begin(); newIter != newPathes.end(); ++newIter)
    {
        for (oldIter = oldPathes.begin(); oldIter != oldPathes.end(); ++oldIter)
        {
            if (*newIter == *oldIter)
            {
                break;
            }
        }

        if (oldIter == oldPathes.end())
        {
            AddSiTable(newIter->c_str());
        }
    }

    for (oldIter = oldPathes.begin(); oldIter != oldPathes.end(); ++oldIter)
    {
        for (newIter = newPathes.begin(); newIter != newPathes.end(); ++newIter)
        {
            if (*oldIter == *newIter)
            {
                break;
            }
        }

        if (newIter == newPathes.end())
        {
            DelSiTable(oldIter->c_str());
        }
    }
}

void Controller::SendUdp(NetworkCfgInterface *network, TsPacketInterface *tsPacket, TableId tableId)
{
#define MaxBufferSize 1024 * 1024 * 64
    static uchar_t buffer[MaxBufferSize];
    int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    NetworkCfgInterface::iterator receiverIter;
    for (receiverIter =  network->Begin(); 
         receiverIter != network->End(); 
         ++receiverIter)
    {
        std::list<TsId> tsIds;
        ReceiverInterface *receiver = *receiverIter;
        ReceiverInterface::iterator tsIdIter;
        for (tsIdIter = receiver->Begin(); tsIdIter != receiver->End(); ++tsIdIter)
        {
            tsIds.push_back(*tsIdIter);
        }

        size_t size = tsPacket->GetCodesSize(tableId, tsIds);    
        assert(size <= MaxBufferSize);
        tsPacket->MakeCodes(receiver->GetReceiverId(), tableId, tsIds, buffer, size);

        int pktNumber = (size + UdpPayloadSize - 1) / UdpPayloadSize;
        struct sockaddr_in socketAddr = receiver->GetSocketAddr();
        for (int i = 0; i < pktNumber; ++i)
        {
            int udpSize = std::min((int)(size - UdpPayloadSize * i), (int)(UdpPayloadSize));
            sendto(socketFd, (char*)buffer + UdpPayloadSize * i, (int)udpSize, 0, 
                   (SOCKADDR *)&socketAddr, 
                   sizeof(struct sockaddr_in));
        }
    }    

    closesocket(socketFd);
}

void Controller::ScheduleTimer(NetId netId, TableId tableId)
{
    TimerArg *timerArg = new TimerArg(netId, tableId);
    TimerRepository::iterator timerIter = timerRepository->Find(*timerArg);
    if (timerIter == timerRepository->End())
    {
        time_t interval = timerCfg->GetInterval(tableId);
        ACE_Time_Value delay(interval);
        ACE_Reactor *reactor = this->reactor();    
        TimerId timerId = reactor->schedule_timer(this, timerArg, ACE_Time_Value::zero, delay);

        timerRepository->Add(timerId, timerArg);
    }
    else
    {
        delete timerArg;
    }
}