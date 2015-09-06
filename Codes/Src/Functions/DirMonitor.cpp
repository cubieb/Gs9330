#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "DirMonitor.h"
using namespace std;

#if defined(_WIN32)

void CALLBACK FileIoCompletionRoutine(DWORD errCode, DWORD numberOfBytesTransfered, LPOVERLAPPED overlapped)
{
    DirMonitorData *dirMonitorData = reinterpret_cast<DirMonitorData*>(overlapped);
    dirMonitorData->dirMonitor->FileIoCompletionRoutine(errCode, numberOfBytesTransfered, overlapped);
}

DirMonitor::DirMonitor(const char *dir, CompletionRoutine routine)
    : dir(dir), routine(routine), dirHandle(INVALID_HANDLE_VALUE), 
      isMonitering(false), dirMonitorData(new DirMonitorData)
{ 
    dirMonitorData->dirMonitor = this;
}

DirMonitor::~DirMonitor()
{
    StopMonitoring();
}

void DirMonitor::RereshMonitoring()
{
    DWORD lastError;
    if (!ReadDirectoryChangesW(dirHandle, dirMonitorData->buffer, DirMonitorData::BufferSize, FALSE,
                                FILE_NOTIFY_CHANGE_FILE_NAME,
                                NULL, &dirMonitorData->overlapped, ::FileIoCompletionRoutine))
    {
        lastError = GetLastError();
        errstrm << "ReadDirectoryChangesW() failed, lastError = " << lastError << std::endl;
    }  
}

void DirMonitor::StartMonitoring()
{
    isMonitering = true;

    dirHandle = CreateFile(dir.c_str(), 
                           FILE_LIST_DIRECTORY,
                           FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 
                           NULL, 
                           OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 
                           NULL); 
    assert(dirHandle != INVALID_HANDLE_VALUE);        
    RereshMonitoring();      
}

void DirMonitor::StopMonitoring()
{
    if (!isMonitering)
        return;

    isMonitering = false;
    CancelIo(dirHandle);

    if (!HasOverlappedIoCompleted(&dirMonitorData->overlapped))
	{
		SleepEx(5, TRUE);
	}
    CloseHandle(dirHandle);
}

void DirMonitor::FileIoCompletionRoutine(DWORD errCode, DWORD numberOfBytesTransfered, LPOVERLAPPED overlapped)
{
    DirMonitorData *dirMonitorData = reinterpret_cast<DirMonitorData*>(overlapped);
    char file[MAX_PATH];

    if (errCode == ERROR_SUCCESS)
    {
        PFILE_NOTIFY_INFORMATION ptr;
        size_t offset = 0;
                    
        do
		{
			ptr = (PFILE_NOTIFY_INFORMATION) &dirMonitorData->buffer[offset];
			offset = offset + ptr->NextEntryOffset;

			int count = WideCharToMultiByte(CP_ACP, 0, ptr->FileName,
			                                ptr->FileNameLength / sizeof(WCHAR),
			                                file, MAX_PATH - 1, NULL, NULL);
			file[count] = '\0';

            if (ptr->Action == FILE_ACTION_ADDED)
            {
                DWORD bytes = 0;
                /*
                MSDN: Returning from this function allows another pending I/O completion routine to be called. 
                All waiting completion routines are called before the alertable thread's wait is completed 
                with a return code of WAIT_IO_COMPLETION. The system may call the waiting completion routines 
                in any order. They may or may not be called in the order the I/O functions are completed.
                */
                routine(file);
            }
		} while (ptr->NextEntryOffset != 0);
    }

    if (isMonitering)
	{
		RereshMonitoring();
	}
}

#endif