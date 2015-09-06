#ifndef _Ts_h_
#define _Ts_h_

#define TsPacketSize 188

#pragma pack(push, 1)
/*
    define struct transport_packet just for calculating fixed fields size.
*/
struct transport_packet
{
    uchar_t  sync_byte:8;                       // 8 uimsbf  -
    uint16_t transport_error_indicator:1;       // 1 bslbf    
    uint16_t payload_unit_start_indicator:1;    // 1 bslbf    
    uint16_t transport_priority:1;              // 1 bslbf
    uint16_t PID:13;                            // 13 bslbf  --
    uchar_t  transport_scrambling_control:2;    // 2 bslbf
    uchar_t  adaptation_field_control:2;        // 2 bslbf
    uchar_t  continuity_counter:4;              // 4 bslbf   -
};
#pragma pack(pop)

class Section;
/**********************class Segment**********************/
class Segment
{
public:
    typedef std::list<uchar_t*>::iterator iterator;
    Segment();

    void Init(std::shared_ptr<Section> section, size_t segmentSize);
    iterator begin();
    iterator end();

    uint_t GetSegmentNumber(std::shared_ptr<Section> section, size_t segmentSize);

private:
    std::shared_ptr<uchar_t> buffer;
    std::list<uchar_t*> segments;
};

/**********************class Ts**********************/
class Ts
{
public:
    Ts(uint16_t pid);

    void SetTransportPriority(uint16_t theTransportPriority);
    void SetContinuityCounter(uchar_t theContinuityCounter);
    uint16_t GetPid();

    size_t GetCodesSize(const std::bitset<256>& tableIds) const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize, const std::bitset<256>& tableIds);

    void AddSection(std::shared_ptr<Section> section);
    void Clear(uint16_t netId);

private:
    struct transporPacket
    {
        uint16_t transportPriority:1;
        uchar_t  adaptationFieldControl:2;
        uchar_t  continuityCounter:4; 
    }transporPacket;
    std::list<std::shared_ptr<Section>> sections;
    uint16_t pid;
};

#endif