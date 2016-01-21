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

template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, size_t>::type
WriteBuffer(uchar_t *buf, T value)
{
    size_t size = value.size();
    memcpy(buf, value.c_str(), value.size());

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
size_t Write(void *dest, size_t destSize, const void *src, size_t count);

#endif
