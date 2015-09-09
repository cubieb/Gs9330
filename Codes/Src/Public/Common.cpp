#include "SystemInclude.h"

#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8
#include <cstdint>        // std::uint_least32_t

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

proposal 2:
void ConvertUtf8ToGb2312(char *to, size_t toSize, char *from)
{
    size_t fromSize = 1024;
    iconv_t cd = iconv_open("gb2312","utf-8");
    iconv(cd, (const char**)&from, &fromSize, &to, &toSize);
    iconv_close(cd);
}

std::pair<shared_ptr<char>, size_t> ConvertUtf8ToGb2312(const std::shared_ptr<xmlChar>& str)
{
    size_t wcharNum = MultiByteToWideChar(CP_UTF8, 0, 
        (char*)str.get(),  // lpMultiByteStr, Pointer to the character string to convert.
        -1,                // cbMultiByte, can be -1, if the string is null-terminated.
        NULL, 0);
    if (wcharNum == 1)
    {
        // just a '\0'
        shared_ptr<char> gb2312(new char[1], CharDeleter());
        gb2312.get()[0] = '\0';
        return make_pair(gb2312, 1);
    }
    size_t strBytes = sizeof(WCHAR) * wcharNum; // 0x13 + gb2312-string + '\0'
    shared_ptr<char> gb2312(new char[strBytes], CharDeleter());
    gb2312.get()[0] = 0x13;
    ConvertUtf8ToGb2312(gb2312.get() + 1, strBytes - 1, (char*)str.get());

    return make_pair(gb2312, strBytes);
}

proposal 3:
    use C++11's lib to convert utf8 to gb2312

Example:
    std::string str  = ConvertUtf8ToString(str("\xe6\xb2\x88\xe9\x98\xb3\xe5\xb8\x82\xe7\xbd\x91\0");
 */
string ConvertUtf8ToString(uchar_t *src)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    wstring wStr = conv.from_bytes(string((char *)src));

    std::locale sysLocale("");
	const wchar_t* dataFrom = wStr.c_str();
	const wchar_t* dataFromEnd = wStr.c_str() + wStr.size();
	const wchar_t* dataFromNext = 0;

    int wcharSize = 4;
	char* dataTo = new char[(wStr.size() + 1) * wcharSize];
	char* dataToNnd = dataTo + (wStr.size() + 1) * wcharSize;
	char* dataToNext = 0;

    memset(dataTo, 0, (wStr.size() + 1) * wcharSize);
    dataTo[0] = 0x13;

    typedef std::codecvt<wchar_t, char, mbstate_t> convertFacet;
	mbstate_t outState = 0;
	auto result = std::use_facet<convertFacet>(sysLocale).out(
		outState, dataFrom, dataFromEnd, dataFromNext,
		dataTo + 1, dataToNnd, dataToNext);
	if( result == convertFacet::ok)
	{
        if (strlen(dataTo) == 1)
            dataTo[0] = 0;
		std::string dst = dataTo;
		delete[] dataTo;
		return dst;
	}
	else
	{
		printf( "convert error!\n" );
		delete[] dataTo;
		return std::string("");
	}
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
  
    time_t ret = mktime(&timeinfo); 
    return ret; 
} 
