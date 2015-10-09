#ifndef _TsDataWrapperInl_h_
#define _TsDataWrapperInl_h_

#include "SystemInclude.h"
#include <regex>
#include "DirMonitor.h"
#include "TsDataWrapper.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

#pragma pack(push, 1)
struct transport_packet
{
    uchar_t sync_byte: 8;

    uint16_t transport_error_indicator: 1;
    uint16_t payload_unit_start_indicator: 1;
    uint16_t transport_priority: 1;
    uint16_t PID: 13;

    uchar_t transport_scrambling_control: 2;
    uchar_t adaptation_field_control: 2;
    uchar_t continuity_counter: 4;
};
#pragma pack(pop)

/**********************class TsDataWrapper**********************/
template<typename Section>
TsDataWrapper<Section>::TsDataWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler,
        const char *tsFileDir, const char *tsFileRegularExp)
    : MyBase(insertHandler, deleteHandler), tsFileDir(tsFileDir), tsFileRegularExp(tsFileRegularExp)
{}

template<typename Section>
TsDataWrapper<Section>::~TsDataWrapper() 
{}   

template<typename Section>
void TsDataWrapper<Section>::Start()
{
    /* linux api  : http://linux.die.net/man/7/inotify
        windows api: ReadDirectoryChangesW() or FileSystemWatcher component
    */
    DirMonitor& dirMonitor = DirMonitor::GetInstance();
    dirMonitor.AddDir(tsFileDir.c_str(), 
        bind(&TsDataWrapper::HandleDbInsert, this, _1),
        bind(&TsDataWrapper::HandleDbDelete, this, _1));
}

//DirMonitor -> XmlDataWrapper<Section>::HandleDbInsert()
template<typename Section>
void TsDataWrapper<Section>::HandleDbInsert(const char *file)
{
    if (!regex_match(file, regex(tsFileRegularExp)))
        return;
    
    CreateSection(file);   //->NotifyDbInsert()
}

//DirMonitor -> XmlDataWrapper<Section>::HandleDbDelete()
template<typename Section>
void TsDataWrapper<Section>::HandleDbDelete(const char *file)
{
    if (!regex_match(file, regex(tsFileRegularExp)))
        return;
    
    NotifyDbDelete(tsFileRegularExp.c_str(), file);
}

/**********************class NitTsWrapper**********************/
template<typename Section>
NitTsWrapper<Section>::NitTsWrapper(DbInsertHandler& insertHandler, 
                                    DbDeleteHandler& deleteHandler, 
                                    const char *tsFileDir)
    : MyBase(insertHandler, deleteHandler, tsFileDir, ".*[nN]it.*\\.ts")
{
}

template<typename Section>
void NitTsWrapper<Section>::CreateSection(const char *file) const
{
    string tsPath = tsFileDir + string("/") + string(file);
    fstream fs(tsPath, ios_base::in  | ios::binary);
    uint16_t pid;
    uchar_t tableId = 0;
    uint16_t payloadSize;
    uchar_t buf[TsPacketSize];
    static uchar_t *sectionBuf = new uchar_t[1024]; /* nit max section_length is 1024 */
    uint16_t sectionLen;
    size_t offset;

    auto section = std::make_shared<Section>(file);
    while (fs.peek() != EOF)
    {
        fs.read((char*)buf, TsPacketSize);
        assert(buf[0] == 0x47);
        Read16(buf + 1, pid);
        pid = pid & 0x1fff;
        if (pid != 0x0010)
        {
            continue;
        }

        uchar_t *ptr;        
        if (buf[sizeof(transport_packet)] == 0x0)
        {
            /* first ts of section start with 0x0 */
            ptr = buf + sizeof(transport_packet) + 1;
            Read8(ptr, tableId);
            Read16(ptr + 1, sectionLen);
            sectionLen = sectionLen & 0x0fff;
            offset = 0;
            payloadSize = TsPacketSize - sizeof(transport_packet) - 1;
        }
        else
        {
            ptr = buf + sizeof(transport_packet);
            payloadSize = TsPacketSize - sizeof(transport_packet);
        }

        if (tableId != 0x40 && tableId != 0x41)
            continue;

        memcpy(sectionBuf + offset, ptr, payloadSize);
        offset = offset + payloadSize;
        assert(offset < 1024);
        if (offset > sectionLen)
        {
            auto sec = std::make_shared<Section>(file, sectionBuf);
            NotifyDbInsert(sec);
        }
    }
}

/**********************class SdtTsWrapper**********************/
template<typename Section>
SdtTsWrapper<Section>::SdtTsWrapper(DbInsertHandler& insertHandler, 
                                    DbDeleteHandler& deleteHandler, 
                                    const char *tsFileDir)
    : MyBase(insertHandler, deleteHandler, tsFileDir, ".*[sS]dt.*\\.ts")
{
}

template<typename Section>
void SdtTsWrapper<Section>::CreateSection(const char *file) const
{
    string tsPath = tsFileDir + string("/") + string(file);
    fstream fs(tsPath, ios_base::in  | ios::binary);
    uint16_t pid;
    uchar_t tableId = 0;
    uint16_t payloadSize;
    uchar_t buf[TsPacketSize];
    static uchar_t *sectionBuf = new uchar_t[1024]; /* nit max section_length is 1024 */
    uint16_t sectionLen;
    size_t offset;

    auto section = std::make_shared<Section>(file);
    while (fs.peek() != EOF)
    {
        fs.read((char*)buf, TsPacketSize);
        assert(buf[0] == 0x47);
        Read16(buf + 1, pid);
        pid = pid & 0x1fff;
        if (pid != 0x0011)
        {
            continue;
        }

        uchar_t *ptr;
        if (buf[sizeof(transport_packet)] == 0x0)
        {
            /* first ts of section start with 0x0 */
            ptr = buf + sizeof(transport_packet) + 1;
            Read8(ptr, tableId);
            Read16(ptr + 1, sectionLen);
            sectionLen = sectionLen & 0x0fff;
            offset = 0;
            payloadSize = TsPacketSize - sizeof(transport_packet) - 1;
        }
        else
        {
            ptr = buf + sizeof(transport_packet);
            payloadSize = TsPacketSize - sizeof(transport_packet);
        }

        if (tableId != 0x42 && tableId != 0x46)
            continue;

        memcpy(sectionBuf + offset, ptr, payloadSize);
        offset = offset + payloadSize;
        assert(offset < 1024);
        if (offset > sectionLen)
        {
            auto sec = std::make_shared<Section>(file, sectionBuf);
            NotifyDbInsert(sec);
        }
    }
}


#endif