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

/* size:  char number, must be multiple of 2 */
size_t ConvertStrToIntStr(const uchar_t* src, size_t size, uchar_t* dst);

inline uchar_t ConvertValueToBcd(uchar_t value)
{
	return( ( ( value / 10 ) << 4 ) | ( value % 10 ) );				/* Convert Value to BCD */
}

size_t ConvertUtf8ToString(uchar_t *src, std::string &dst, size_t maxCharNumber);
size_t ConvertUtf8ToString(uchar_t *src, std::string &dst);

#endif