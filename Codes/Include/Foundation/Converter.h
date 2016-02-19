#ifndef _Converter_h_
#define _Converter_h_

/******************function ConvertCharToInt******************/
uchar_t ConvertCharToInt(uchar_t input);
size_t GetUtfMemSize(uchar_t *src, size_t charNumber);

template <typename T>
typename std::enable_if<std::is_integral<T>::value, size_t>::type
ConvertHexStrToInt(uchar_t *src, T &value)
{
    value = 0;
    
    for (uchar_t *ptr = src; ptr < src + sizeof(T) * 2; ptr = ptr + 2)
    {
        value = (value << 8) + (ConvertCharToInt(ptr[0]) << 4) + ConvertCharToInt(ptr[1]);
    }

    return sizeof(T) * 2;
}

/* size:  char number(strlen(src)), must be multiple of 2 
   Example:
     size_t size = ConvertStrToIntStr((uchar_t*)"1112", 4, dst);
    Result:
        size = 2;
        dst = "\x11\x12"
 */
size_t ConvertStrToIntStr(const uchar_t* src, size_t size, uchar_t* dst);

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
/* Note 1: libxml2's API "xmlGetProp()" always return utf8 codes. 
   Note 2:  code table reference "http://wenku.baidu.com/view/fb5f9cb765ce050876321323.html?re=view"
     GB     Unicode   UTF-8     Chinese Character
     ----   -------   --------  -----------------
     D2BB   4E00      E4 B8 80  Ò»
     B6A1   4E01      E4 B8 81  ¶¡
     C6DF   4E03      E4 B8 83  Æß
     CDF2   4E07      E4 B8 87  Íò
 */
size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, size_t maxCharNumber, bool doFirstByte);
size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, bool doFirstByte);

#endif