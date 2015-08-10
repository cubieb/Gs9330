#ifndef _Ts_h_
#define _Ts_h_
#include "SystemInclude.h"

#define TsPacketSize 188

#pragma pack(push, 1)
/*
    define struct transport_packet just for calculating fixed fields size.
*/
struct transport_packet
{
    uint32_t sync_byte:8;                       // 8 uimsbf  -
    uint32_t transport_error_indicator:1;       // 1 bslbf    
    uint32_t payload_unit_start_indicator:1;    // 1 bslbf    
    uint32_t transport_priority:1;              // 1 bslbf
    uint32_t PID:13;                            // 13 bslbf  --
    uint32_t transport_scrambling_control:2;    // 2 bslbf
    uint32_t adaptation_field_control:2;        // 2 bslbf
    uint32_t continuity_counter:4;              // 4 bslbf   -
};
#pragma pack(pop)

class SectionBase;
class Segment;
/**********************class Segment**********************/
class Segment
{
public:
    typedef std::list<uchar_t*>::iterator iterator;
    Segment(const SectionBase& section, size_t segmentSize);

    iterator begin();
    iterator end();

private:
    Segment();
    std::shared_ptr<uchar_t> buffer;
    std::list<uchar_t*> segments;
};

/**********************class Ts**********************/
class Ts
{
public:
    size_t GetCodesSize(const SectionBase& section) const;
    size_t MakeCodes(const SectionBase& section, uchar_t *buffer, size_t bufferSize) const;
};

#endif