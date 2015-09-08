#ifndef _DirMonitor_h_
#define _DirMonitor_h_

#if defined(_WIN32)

typedef std::function<void (const char*)> CompletionRoutine;

/**********************class MonitorState**********************/
class MonitorState
{
public:
    enum { BufferSize = 2048 };

public:
    MonitorState(const char *dir);
    ~MonitorState();

    std::shared_ptr<char>  buffer;
    std::string            dir;
    ULONG                  completionKey;
    std::list<CompletionRoutine>  insertHandlers;
    std::list<CompletionRoutine>  deleteHandlers;

    HANDLE     dirHandle;   //folder under monitoring
    HANDLE     ioPortHandle;
    OVERLAPPED overlapped;

    std::thread     monitorThread;
};

/**********************class DirMonitor**********************/
class DirMonitor
{
public:    
    ~DirMonitor();

    void AddDir(const char *dir, CompletionRoutine insertHandler, CompletionRoutine deleteHandlers);
    void RemoveDir(const char *dir);

    static DirMonitor& GetInstance()
    {
        static DirMonitor instance;
        return instance;
    }

private:
    DirMonitor();
    void ThreadRoutine(std::shared_ptr<MonitorState> state);

private:
    std::map<std::string, std::shared_ptr<MonitorState>> states;
};

#endif
#endif