#ifndef _TsPacket_h_
#define _TsPacket_h_

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

/**********************class TsPacket**********************/
class TsPacket: public TsPacketInterface
{
public:
    TsPacket(NetId netId, Pid pid);
    virtual ~TsPacket();

    void AddSiTable(SiTableInterface *siTable);
    void DelSiTable(TableId tableId, uint16_t key);

    SiTableInterface * FindSiTable(TableId tableId, uint16_t key);
    size_t GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const;
    NetId  GetNetId() const;
    Pid    GetPid() const;
    size_t MakeCodes(uint_t ccId, TableId tableId, const std::list<TsId>& tsIds, 
                     uchar_t *buffer, size_t bufferSize);

private:
    uint_t GetSegmentNumber(size_t codesSize) const;

private:
    std::list<SiTableInterface *> siTables;

    uchar_t  adaptationFieldControl;
    /* the same TsPacket will be sent to multipule socket-addr, so 
     * we need save one continuity_counter per socket-addr.
     * map<continuity-counter-id, continuity-counter>
    */
    std::map<uint_t, uchar_t> continuityCounters;

    NetId    netId;
    Pid      pid;
    uint16_t transportPriority;
};

/**********************class TsPackets**********************/
class TsPackets: public TsPacketsInterface
{
public:
    TsPackets();
    virtual ~TsPackets();

    void Add(TsPacketInterface *tsPacket);
    iterator Begin();
    void Delete(NetId netId, Pid pid);
    iterator End();
    iterator Find(NetId netId, Pid pid);
    NodePtr GetMyHead();

private:
    std::list<TsPacketInterface*> tsPackets;
};

#endif