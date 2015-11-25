#ifndef _Common_h_
#define _Common_h_

/******************Read from / Write to packet buffer******************/
template <typename T>
typename std::enable_if<std::is_integral<T>::value, size_t>::type
ReadBuffer(uchar_t *buf, T &value)
{
    value = 0;
    size_t size = sizeof(T);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, size_t>::type
WriteBuffer(uchar_t *buf, T value)
{
    size_t size = sizeof(T);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = (value >> 4) >> 4;  // value >> 8 will cause C4333 warning in VC 2012.
    }
    return size;
}

size_t Read8(uchar_t* buf, uchar_t&);
size_t Read16(uchar_t* buf, uint16_t&);
size_t Read32(uchar_t* buf, uint32_t&);
size_t Read64(uchar_t* buf, uint64_t&);

size_t Write8(uchar_t* buf, uchar_t);
size_t Write16(uchar_t* buf, uint16_t);
size_t Write32(uchar_t* buf, uint32_t);
size_t Write64(uchar_t* buf, uint64_t);

size_t MemCopy(void *dest, size_t destSize, const void *src, size_t count);

/******************shared_ptr<...> deleter******************/
/*
CharDeleter, auxiliary class used by shared_ptr<char>.  
Example:
{
    shared_ptr<char> buffer(new char[128], CharDeleter());
}
*/
class CharDeleter
{
public:
    CharDeleter()
    {}

    void operator()(char *ptr) const
    {
        delete[] ptr;
    }
};

class UcharDeleter
{
public:
    UcharDeleter()
    {}

    void operator()(uchar_t *ptr) const
    {
        delete[] ptr;
    }
};

/******************function ConvertCharToInt******************/
uchar_t ConvertCharToInt(uchar_t input);

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

size_t ConvertStrToIntStr(const uchar_t* src, size_t size, uchar_t* dst);

inline uchar_t ConvertValueToBcd(uchar_t value)
{
	return( ( ( value / 10 ) << 4 ) | ( value % 10 ) );				/* Convert Value to BCD */
}

size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, size_t maxCharNumber);
size_t ConvertUtf8ToString(uchar_t *src, std::string &dst);

/******************convert string to time_t******************/
time_t ConvertStrToTime(const char *str);
void ConvertUtcToGmt(struct tm& src, struct tm& dst);
void ConvertGmtToUtc(struct tm& src, struct tm& dst);
time_t CalculateUtcGmtDiff();

#endif
