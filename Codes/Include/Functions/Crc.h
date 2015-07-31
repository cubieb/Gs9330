
#ifndef _Crc_h_
#define _Crc_h_

uint32_t ALGO_Crc32(uint32_t crc32, uchar_t *buffer, uint32_t size);
uint32_t CalculateCrc32(uchar_t *buffer, size_t size);

#endif /* _Crc_h_ */