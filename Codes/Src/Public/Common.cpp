#include "SystemInclude.h"

#include "iconv.h"
#include "Common.h"

using namespace std;

Mac::Mac(): mac(new uchar_t[6])
{
    memcpy(this->mac.get(), MacZero, 6);
}

Mac::Mac(const uchar_t* mac): mac(new uchar_t[6])
{
    memcpy(this->mac.get(), mac, 6);
}

Mac::Mac(Mac const& right): mac(right.mac)
{}

Mac const& Mac::operator =(Mac const& right)
{
    mac = right.mac;
    return *this;
}

bool Mac::IsBroadcast() const
{
    return (memcmp(MacBroadcast, mac.get(), 6) == 0);
}

bool Mac::IsZero() const
{
    return (memcmp(MacZero, mac.get(), 6) == 0);    
}

uchar_t* Mac::GetPtr() const
{
    return mac.get();
}

int Mac::Compare(Mac const& right) const
{
    return memcmp(mac.get(), right.mac.get(), 6);
}

int Mac::Compare(const uchar_t* right) const
{
    return memcmp(mac.get(), right, 6);
}

void Mac::Put(ostream& os) const
{
    os << MemStream<uchar_t>(mac.get(),  6);
}

/******************Read from / Write to packet buffer******************/
size_t Read8(uchar_t* buf, uchar_t& value)
{
    value = buf[0];
    return sizeof(uchar_t);
}

size_t Read16(uchar_t* buf, uint16_t& value)
{
    value = 0;
    size_t size = sizeof(uint16_t);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

size_t Read32(uchar_t* buf, uint32_t& value)
{
    value = 0;
    size_t size = sizeof(uint32_t);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

size_t Read64(uchar_t* buf, uint64_t& value)
{
    value = 0;
    size_t size = sizeof(uint64_t);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

size_t Write8(uchar_t* buf, uchar_t value)
{
    buf[0] = value;
    return sizeof(uchar_t);
}

size_t Write16(uchar_t* buf, uint16_t value)
{
    size_t size = sizeof(uint16_t);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = value >> 8;
    }
    return size;
}

size_t Write32(uchar_t* buf, uint32_t value)
{
    size_t size = sizeof(uint32_t);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = value >> 8;
    }
    return size;
}

size_t Write64(uchar_t* buf, uint64_t value)
{
    size_t size = sizeof(uint64_t);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = value >> 8;
    }
    return size;
}

size_t MemCopy(void *dest, size_t destSize, const void *src, size_t count)
{
    assert(destSize >= count);
    memcpy(dest, src, count);
    return count;
}

/******************function ConvertChar2Asc******************/
/* 
proposal 1: without icon lib, we can call windows function to do the same thing.
int len = MultiByteToWideChar(CP_UTF8, 0, (char*)data, -1, NULL, 0);
wchar_t* wstr = new wchar_t[len+1];

MultiByteToWideChar(CP_UTF8, 0, (char*)data, -1, wstr, len);
len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
char* str = new char[len+1];
WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
 */
void ConvertUtf8ToGb2312(char *to, size_t toSize, char *from)
{
    size_t fromSize = 1024;
    iconv_t cd = iconv_open("gb2312","utf-8");
    iconv(cd, (const char**)&from, &fromSize, &to, &toSize);
    iconv_close(cd);
}

/******************convert string to time_t******************/
time_t ConvertStrToTime(const char *str)  
{  
    tm timeinfo;  
    int year = 0, month = 0, day = 0, hour, minute, second;
    if (strchr(str, '-') == nullptr)
        sscanf(str,"%d:%d:%d", &hour, &minute, &second); 
    else
        sscanf(str,"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);  

    timeinfo.tm_year  = year-1900;  
    timeinfo.tm_mon   = month-1;  
    timeinfo.tm_mday  = day;  
    timeinfo.tm_hour  = hour;  
    timeinfo.tm_min   = minute;  
    timeinfo.tm_sec   = second;  
    timeinfo.tm_isdst = 0;  
  
    time_t ret = mktime(&timeinfo); //¨°??-??¨¢?8??¨º¡À??  
    return ret; //??¨º¡À??  
} 
