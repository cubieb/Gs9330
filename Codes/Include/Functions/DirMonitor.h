#ifndef _DirMonitor_h_
#define _DirMonitor_h_

#if defined(_WIN32)

class DirMonitor;
struct DirMonitorData
{
    enum: uint32_t { BufferSize = 1024 * 16 };
	OVERLAPPED overlapped;
	char       buffer[BufferSize];
    DirMonitor *dirMonitor;
};

void CALLBACK FileIoCompletionRoutine(DWORD errCode, DWORD numberOfBytesTransfered, LPOVERLAPPED overlapped);

class DirMonitor
{
public:
    typedef std::function<void (const char*)> CompletionRoutine;

    DirMonitor(const char *dir, CompletionRoutine routine);
    ~DirMonitor();

    void RereshMonitoring();
    void StartMonitoring();
    void StopMonitoring();
    void FileIoCompletionRoutine(DWORD errCode, DWORD numberOfBytesTransfered, LPOVERLAPPED overlapped);

private:
    std::string dir;
    CompletionRoutine routine;
    HANDLE dirHandle;

    bool isMonitering;
    std::shared_ptr<DirMonitorData> dirMonitorData;
};

#endif
#endif