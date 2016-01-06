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
size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, size_t maxCharNumber)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    //utf-8 -> unicode
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
    //unicode -> GB2312
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
