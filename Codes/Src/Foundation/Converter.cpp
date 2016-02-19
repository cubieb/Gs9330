#include "Include/Foundation/SystemInclude.h"
#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8

#include "Include/Foundation/Converter.h"
using namespace std;

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
    assert(strlen((char*)src) >= size);
    for(const uchar_t *ptr = src; ptr < src + size; ptr = ptr + 2)
    {
        *(dst++) = (ConvertCharToInt(ptr[0]) << 4) | ConvertCharToInt(ptr[1]);
    }
    return size / 2;
}

size_t GetUtfMemSize(uchar_t *src, size_t charNumber)
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

size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, size_t maxCharNumber, bool doFirstByte)
{
    if (maxCharNumber == 0)
    {
        dst = "";
        return 0;
    }

    static std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    //utf-8 -> unicode
    wstring wStr = conv.from_bytes((const char *)src);   

    size_t charNumber = std::min(wStr.size(), maxCharNumber);

    /* we use vc to analysis performance , and find out it is  time-consuming to construct 
       a std::locale, so we define it as static variable.
     */
    static std::locale sysLocale("");
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
    //unicode -> GB2312
    size_t offset = doFirstByte ? 1 : 0;
	auto result = std::use_facet<convertFacet>(sysLocale).out(
		outState, dataFrom, dataFromEnd, dataFromNext,
		dataTo + offset, dataToEnd, dataToNext);
	if( result == convertFacet::ok)
	{
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

size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, bool doFirstByte)
{
    return ConvertUtf8ToString(src, dst, std::numeric_limits<size_t>::max(), doFirstByte);
}
