
#ifndef _Crc32_h_
#define _Crc32_h_

class Crc32
{
public:
    Crc32(void);
    ~Crc32(void);

    void Initialize(void);

    bool FileCrc(const char *fileName, uint32_t *crc);
    bool FileCrc(const char *fileName, uint32_t *crc, size_t bufferSize);
    uint32_t FileCrc(const char *fileName);
    uint32_t FileCrc(const char *fileName, size_t bufferSize);

    uint32_t FullCrc(const uchar_t *buffer, size_t bufferSize);
    void FullCrc(const uchar_t *buffer, size_t bufferSize, uint32_t *crc);

    void PartialCrc(uint32_t *crc, const uchar_t *buffer, size_t bufferSize);

private:
    uint32_t Reflect(uint32_t reflect, const char ch);
    uint32_t table[256]; // CRC lookup table array.
};

#endif /* _Crc32_h_ */