#include "Include/Foundation/SystemInclude.h"
#include <regex>
#pragma warning(push)
#pragma warning(disable:702)   //disable warning caused by ACE library.
#pragma warning(disable:4251)  //disable warning caused by ACE library.
#pragma warning(disable:4996)  //disable warning caused by ACE library.
#include "ace/OS.h"
#include "ace/Singleton.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/Time.h"

/* ConfigurationWrapper */
#include "Include/ConfigurationWrapper/TimerCfgWrapperInterface.h"
#include "Include/ConfigurationWrapper/DirCfgWrapperInterface.h"
#include "Include/ConfigurationWrapper/NetworkCfgWrapperInterface.h"
#include "Include/ConfigurationWrapper/NetworkRelationWrapperInterface.h"

/* SiTableWrapper */
#include "Include/SiTableWrapper/SiTableXmlWrapperInterface.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TransportPacketInterface.h"

/* Controller */
#include "TimerRepository.h"
#include "Controller.h"
using namespace std;

/**********************SiTableXmlWrapperRepository**********************/
static SiTableXmlWrapperAutoRegisterSuite<SiTableInterface> batWrapper
    (string("bat"), new BatXmlWrapper<SiTableInterface>);

static SiTableXmlWrapperAutoRegisterSuite<SiTableInterface> eitWrapper
    (string("eit"), new EitXmlWrapper<SiTableInterface>);

static SiTableXmlWrapperAutoRegisterSuite<SiTableInterface> nitWrapper
    (string("nit"), new NitXmlWrapper<SiTableInterface>);

static SiTableXmlWrapperAutoRegisterSuite<SiTableInterface> sdtWrapper
    (string("sdt"), new SdtXmlWrapper<SiTableInterface>);

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
    fileSummaries.clear();
    
    /* cancle timers */
    TimerRepository::iterator iter;
    for (iter = timerRepository->Begin(); iter != timerRepository->End(); ++iter)
    {
        reactor->cancel_timer(iter->first);
    }
    delete timerRepository;    
    delete tsPackets;

    /* free configuration */
    delete dirCfg;
    delete networkCfgs;
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
	while((ACE_OS::access(ok.c_str(), F_OK)) != 0)
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

    TransportPacketsInterface::iterator tsPacketIter = tsPackets->Find(netId, pid);
    assert(tsPacketIter != tsPackets->End());
    (*tsPacketIter)->RefreshCatch();

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

#define TestReadXmlPerformance
bool Controller::Start(ACE_Reactor *reactor, const char *cfgPath)
{
    error_code errCode;
    /********* Step 1: Init Configuration *********/
    /* dir configuration */
    dirCfg = DirCfgInterface::CreateInstance();
    DirCfgWrapperInterface<DirCfgInterface> dirCfgWrapper;
    errCode = dirCfgWrapper.Select(*dirCfg, cfgPath);
    if (errCode)
    {
        errstrm << "Error when reading " << cfgPath << ", error message: " << errCode.message() << endl;
        return false;
    }
    
    char *cfgDir = ACE_OS::getenv("EpgSenderData");    
    if (cfgDir == nullptr)
    {
        errstrm << "Error when get environment variable of EpgSenderData" << endl;
        return false;
    }

    /* networks configuration */
    string receiverCfgPath = string(cfgDir) + string("\\receiver.xml");
    networkCfgs = NetworkCfgsInterface::CreateInstance();
    NetworkCfgWrapperInterface<NetworkCfgsInterface, NetworkCfgInterface, ReceiverInterface> networkCfgWrapper;
    errCode = networkCfgWrapper.Select(*networkCfgs, receiverCfgPath.c_str());
    if (errCode)
    {
        cout << "Error when reading " << receiverCfgPath << ", error message: " << errCode.message() << endl;
        return false;
    }

    /* timer configuration */
    string senderCfgPath = string(cfgDir) + string("\\sender.xml");
    timerCfg = TimerCfgInterface::CreateInstance();
    TimerCfgWrapperInterface<TimerCfgInterface> timerCfgWrapper;
    errCode = timerCfgWrapper.Select(*timerCfg, senderCfgPath.c_str());
    if (errCode)
    {
        errstrm << "Error when reading " << senderCfgPath << ", error message: " << errCode.message() << endl;
        return false;
    }

    /********* Step 2: Init Reactor *********/
    this->reactor (reactor);    

    /********* Step 3: Setup Runtime Information *********/
    /* this->tsPackets */
    tsPackets = TransportPacketsInterface::CreateInstance();    

    /* Timer Repository(timer runtimer information) */
    timerRepository = new TimerRepository();    

#ifdef TestReadXmlPerformance
    TimeMeter timeMeter;
    timeMeter.Start();

    ReadDir(dirCfg->GetXmlDir());

    timeMeter.End();
    cout << "elapsed time(milliseconds): " << timeMeter.GetDuration().count() << endl;
#else
    ReadDir(dirCfg->GetXmlDir());
#endif

    AddMonitoredDir(dirCfg->GetXmlDir());
    //AddMonitoredDir(dirCfg->GetTsDir()); 
    return true;
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
    error_code errCode = netRelationWrapper.Select(*networkCfgs, relationXmlPath.c_str());
    if (errCode)
    {
        errstrm << "Error when reading " << relationXmlPath 
             << ", error message: " << errCode.message() << endl;
        return;
    }
        
    /* Add SiSable to TsPacket */
    TransportPacketInterface *tsPacket;
    TransportPacketsInterface::iterator iter = tsPackets->Find(netId, pid);
    if (iter == tsPackets->End())
    {
        tsPacket = TransportPacketInterface::CreateInstance(netId, pid);
        tsPackets->Add(tsPacket);
    }
    else
    {
        tsPacket = *iter;
    }

    typedef SiTableXmlWrapperRepository<SiTableInterface> Repository;
    typedef SiTableXmlWrapperInterface<SiTableInterface> Wrapper;

    Repository &repository = Repository::GetInstance();
    Wrapper &siTableWrapper = repository.GetWrapperInstance(type);
    list<SiTableInterface*> siTables = siTableWrapper.Select(path);
    
    /* save {file name, table id, key list} relation ship */
    FileSummary summary(path);
    list<SiTableInterface*>::iterator ii;
    for (ii = siTables.begin(); ii != siTables.end(); ++ii)
    {
        TableId    tableId  = (*ii)->GetTableId();
        SiTableKey tableKey = (*ii)->GetKey();
        SiTableIdAndKey tableIdAndKey = FileSummary::AssembleTableIdAndKey(tableId, tableKey);
        summary.tableIdAndKeys.push_back(tableIdAndKey);
        
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
                
        tsPacket->AddSiTable(*ii);
    }
    fileSummaries.push_back(move(summary));

}

void Controller::DelSiTable(const char *path)
{
    NetId   netId;
    Pid     pid;
    string  type;
    cout << "Removing " << path << endl;
    AnalyzeFileName(path, netId, pid, type);   

    TransportPacketsInterface::iterator iter = tsPackets->Find(netId, pid);
    assert(iter != tsPackets->End());
    TransportPacketInterface *tsPacket = *iter;

    std::list<FileSummary>::iterator summaryIter;
    summaryIter = find_if(fileSummaries.begin(), fileSummaries.end(), CompareSummaryFileName(path));
    assert (summaryIter != fileSummaries.end());
    list<SiTableIdAndKey>::iterator ii;
    for (ii = summaryIter->tableIdAndKeys.begin(); ii != summaryIter->tableIdAndKeys.end(); ++ii)
    {
        TableId tableId = FileSummary::GetTableId(*ii);
        SiTableKey tableKey = FileSummary::GetTableKey(*ii);

        tsPacket->DelSiTable(tableId, tableKey);
    }
    fileSummaries.remove_if(CompareSummaryFileName(path));
}

void Controller::ReadDir(const char *dir)
{    
    list<string> newPathes, oldPathes;
    
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

    list<FileSummary>::iterator summeryIter;
    for (summeryIter = fileSummaries.begin(); summeryIter != fileSummaries.end(); ++summeryIter)
    {
        oldPathes.push_back(summeryIter->fileName);
    }

    vector<string> added, deled;
    
    newPathes.sort();
    oldPathes.sort();
    set_difference(newPathes.begin(), newPathes.end(), 
                   oldPathes.begin(), oldPathes.end(), 
                   std::back_inserter(added));
    for (vector<string>::iterator iter = added.begin(); iter != added.end(); ++iter)
    {
        AddSiTable(iter->c_str());
    }

    set_difference(oldPathes.begin(), oldPathes.end(), 
                   newPathes.begin(), newPathes.end(), 
                   std::back_inserter(deled));
    for (vector<string>::iterator iter = deled.begin(); iter != deled.end(); ++iter)
    {
        DelSiTable(iter->c_str());
    }
}

void Controller::SendUdp(NetworkCfgInterface *network, 
                         TransportPacketInterface *tsPacket, 
                         TableId tableId)
{
    static size_t bufferSize = 1024 * 1024 * 16;
    static uchar_t *buffer = new uchar_t[bufferSize];
    
    int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    NetworkCfgInterface::iterator receiverIter;
    for (receiverIter =  network->Begin(); 
         receiverIter != network->End(); 
         ++receiverIter)
    {
        ReceiverInterface *receiver = *receiverIter;
        if (receiverIter != network->Begin())
        {
            /* bat and sdt will only be sent to center frequency. */
            if (tableId == BatTableId 
                || tableId == NitActualTableId || tableId == NitOtherTableId)
            {
                break;
            }
        }

        TsId tsId = receiver->GetTsId();
        size_t size = tsPacket->GetCodesSize(tableId, tsId); 
        
        if (size > bufferSize)
        {
            bufferSize = bufferSize * 2;
            delete[] buffer;
            buffer = new uchar_t[bufferSize];
            assert(bufferSize <= 1024*1024*512);
        }
        /* Ts is is unique for every receiver, 
           So we use Receiver's tsId as ccId index.
         */
        tsPacket->MakeCodes(tsId, tableId, tsId, buffer, size);

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

#pragma warning(pop)
