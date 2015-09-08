#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "DirMonitor.h"
using namespace std;

#if defined(_WIN32)
/**********************class MonitorState**********************/
MonitorState::MonitorState(const char *dir)
    : buffer(new char[BufferSize], CharDeleter()), dir(dir), 
      completionKey(1)
{ 
    dirHandle = CreateFile(dir, 
        FILE_LIST_DIRECTORY, 
        FILE_SHARE_READ | FILE_SHARE_WRITE ,
        NULL, //security attributes
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, //OVERLAPPED!
        NULL);
    assert(dirHandle != INVALID_HANDLE_VALUE);
        
    ioPortHandle = CreateIoCompletionPort(dirHandle, NULL, (ULONG_PTR)&completionKey, 1);
    assert( ioPortHandle != NULL);   
}

MonitorState::~MonitorState()
{    
    CloseHandle(ioPortHandle);
    CloseHandle(dirHandle);
}

/**********************class DirMonitor**********************/
DirMonitor::DirMonitor()
{}
   
DirMonitor::~DirMonitor()
{}

void DirMonitor::AddDir(const char *dir, CompletionRoutine insertHandler, CompletionRoutine deleteHandlers)
{
    auto iter = states.find(dir);
    if (iter == states.end())
    {
        shared_ptr<MonitorState> state = make_shared<MonitorState>(dir);
        state->insertHandlers.push_back(insertHandler);
        state->deleteHandlers.push_back(deleteHandlers);
        states.insert(make_pair(dir, state));

        state->monitorThread = thread(&DirMonitor::ThreadRoutine, this, state);
    }
    else
    {
        iter->second->insertHandlers.push_back(insertHandler);
        iter->second->deleteHandlers.push_back(deleteHandlers);
    }    
}

void DirMonitor::RemoveDir(const char *dir)
{
    auto iter = states.find(dir);
    if (iter == states.end())
        return;

    shared_ptr<MonitorState> state = iter->second;
    PostQueuedCompletionStatus(state->ioPortHandle, 0, 0, NULL);
    state->monitorThread.join();
    states.erase(iter);
}

void DirMonitor::ThreadRoutine(shared_ptr<MonitorState> state)
{
    BOOL resultl, resultr;
    DWORD bytes;
    ULONG_PTR    completionKey;
    LPOVERLAPPED lpoverlapped;
    char file[MAX_PATH];

    while (true)
    {
        memset(&state->overlapped, 0, sizeof(OVERLAPPED));
        resultl = ReadDirectoryChangesW(state->dirHandle, state->buffer.get(), 
            MonitorState::BufferSize, FALSE,  FILE_NOTIFY_CHANGE_FILE_NAME,
            NULL, &state->overlapped, NULL);

        resultr = GetQueuedCompletionStatus(state->ioPortHandle, &bytes, 
            &completionKey, &lpoverlapped, INFINITE);

        if (!resultl || !resultr)
        {
            cout << "error" << endl;
            continue;
        }

        /* will be NULL if I call PostQueuedCompletionStatus(state->ioPortHandle, 0, 0, NULL); */
        if (completionKey == NULL)
            break;

        FILE_NOTIFY_INFORMATION* notifyPtr;
        size_t offset = 0;
        do
        {
            notifyPtr = (FILE_NOTIFY_INFORMATION*)&state->buffer.get()[offset];
            offset = offset + notifyPtr->NextEntryOffset;
            
            int count = WideCharToMultiByte(CP_ACP, 0, notifyPtr->FileName,
                notifyPtr->FileNameLength / sizeof(WCHAR),
                file, MAX_PATH - 1, NULL, NULL);
            file[count] = '\0';
                       
            if (notifyPtr->Action == FILE_ACTION_ADDED)
            {
                for (auto handler: state->insertHandlers)
                {
                    handler(file);
                }
            }
            else if (notifyPtr->Action == FILE_ACTION_REMOVED)
            {                
                for (auto handler: state->deleteHandlers)
                {
                    handler(file);
                }
            }
        }while (notifyPtr->NextEntryOffset != 0);
    } 
}

#endif