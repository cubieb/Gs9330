#ifndef _Crc32_h_
#define _Crc32_h_

class Crc32
{
public:
    Crc32();
    ~Crc32();

    uint32_t CalculateCrc(const uchar_t *buffer, size_t bufferSize);
};

#endif /* _Crc32_h_ */