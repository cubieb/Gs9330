#include "SystemInclude.h"

#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8
#include <cstdint>        // std::uint_least32_t

#include "Common.h"

using namespace std;

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

/******************function ConvertCharToInt******************/
uchar_t ConvertCharToInt(uchar_t input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    
    assert(input >= 'a' && input <= 'f');
    return input - 'a' + 10;
}

size_t ConvertStrToIntStr(const uchar_t* src, size_t size, uchar_t* dst)
{
    assert((size & 1) == 0);
    for(const uchar_t *ptr = src; ptr < src + size; ptr = ptr + 2)
    {
        *(dst++) = (ConvertCharToInt(ptr[0]) << 4) | ConvertCharToInt(ptr[1]);
    }
    return size / 2;
}


/* Convert Utf8 sting to Gb2312 string.

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
static size_t GetUtfMemSize(uchar_t *src, size_t charNumber)
{
    size_t chars = 0, memSize = 0;
    while(true)
    {
        if ((src[0] & 0xC0) != 0x80)
        {
            chars++;
        }

        if (chars == charNumber + 1)
            break;

        src++;
        memSize++;
    }
    return memSize;
}

size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, size_t maxCharNumber)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    wstring wStr = conv.from_bytes(string((char *)src));   

    size_t charNumber = std::min(wStr.size(), maxCharNumber);

    std::locale sysLocale("");
	const wchar_t* dataFrom = wStr.c_str();
	const wchar_t* dataFromEnd = wStr.c_str() + charNumber;
	const wchar_t* dataFromNext = 0;

    int wcharSize = 4;
	char* dataTo = new char[(wStr.size() + 1) * wcharSize];
	char* dataToEnd = dataTo + (wStr.size() + 1) * wcharSize;
	char* dataToNext = 0;

    memset(dataTo, 0, (wStr.size() + 1) * wcharSize);
    dataTo[0] = 0x13;

    typedef std::codecvt<wchar_t, char, mbstate_t> convertFacet;
	mbstate_t outState = 0;
	auto result = std::use_facet<convertFacet>(sysLocale).out(
		outState, dataFrom, dataFromEnd, dataFromNext,
		dataTo + 1, dataToEnd, dataToNext);
	if( result == convertFacet::ok)
	{
        if (strlen(dataTo) == 1)
            dataTo[0] = 0;
		dst = dataTo;
	}
	else
	{
		cerr << "convert error!" << endl;
        dst = "";
	}

    delete[] dataTo;
    return GetUtfMemSize(src, charNumber);
}

size_t ConvertUtf8ToString(uchar_t *src, std::string &dst)
{
    return ConvertUtf8ToString(src, dst, std::numeric_limits<size_t>::max() );
}

/******************convert string to time_t******************/
/* to convert struct tm to string, use std::put_time().
   consider use std::get_time, std::time_get::get_time, std::time_get_byname::get_time 
   to impliment current function.
   exsample:
        std::tm t = {};
        std::istringstream ss("2011-5-18 23:12:34");
        ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

    if use sscanf() to convert string to time:
        sscanf(str,"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);  
 */
time_t ConvertStrToTime(const char *str)  
{  
    tm timeinfo;  
    std::istringstream ss(str);
    ss >> std::get_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
  
    time_t ret = mktime(&timeinfo); 
    return ret; 
} 

void ConvertUtcToGmt(struct tm& src, struct tm& dst)
{
    time_t time = std::mktime(&src);
    dst = *std::gmtime(&time);
}

/* refer to http://www.cplusplus.com/reference/ctime/gmtime/ */
#define CCT (+8)
void ConvertGmtToUtc(struct tm& src, struct tm& dst)
{
    dst = src;
    dst.tm_hour = (src.tm_hour + CCT) % 24;
}

time_t CalculateUtcGmtDiff()
{    
    time_t gmt = std::time(nullptr);
    struct tm calendar ;
    calendar  = *std::gmtime(&gmt);
    time_t utc = std::mktime(&calendar);
    time_t diff = (time_t)difftime(utc, gmt);
    return diff;
}