#ifndef _SiTableInterface_h_
#define _SiTableInterface_h_

/**********************class SiTableInterface**********************/
class SiTableInterface
{
public: 
    SiTableInterface(){};
    virtual ~SiTableInterface(){};
    
    virtual size_t GetCodesSize(TableId tableId, const std::list<TsId> &tsIds) const = 0;
    //nit: network_id, sdt: transport_stream_id, bat: bouquet_id, eit: service_id
    virtual uint16_t GetKey() const = 0;  
    //table type: nit 0x41, 0x41; sdt 0x42, 0x46;  bat 0x4a; eit 0x4e, 0x4f, 0x50, 0x60
    virtual TableId GetTableId() const = 0;  
    virtual size_t MakeCodes(TableId tableId, std::list<TsId> &tsIds, uchar_t *buffer, size_t bufferSize) const = 0;
};

class CompareSiTableIdAndKey: public std::unary_function<SiTableInterface, bool>
{
public:
    CompareSiTableIdAndKey(TableId tableId, uint16_t key)
        : tableId(tableId), key(key)
    {}

    result_type operator()(const argument_type &siTable)
    {
        return (result_type)(siTable.GetTableId() == tableId && siTable.GetKey() == key);
    }    

    result_type operator()(const argument_type *siTable)
    {
        return this->operator()(*siTable);
    }

private:
    TableId  tableId;
    uint16_t key;
};

/**********************class BatTableInterface**********************/
class BatTableInterface: public SiTableInterface
{
public:
    BatTableInterface(){};
    virtual ~BatTableInterface(){};
    
    virtual void AddDescriptor(std::string &data) = 0;
    virtual void AddTs(TsId tsId, OnId onId) = 0;
    virtual void AddTsDescriptor(TsId tsId, std::string &data) = 0; 
    
    static BatTableInterface * CreateInstance(TableId tableId, BouquetId bouquetId,  Version versionNumber);
};

/**********************class EitTableInterface**********************/
class EitTableInterface: public SiTableInterface
{
public:
    EitTableInterface(){};
    virtual ~EitTableInterface(){};

    virtual void AddEvent(EventId eventId, const char *startTime, time_t duration,
                          uint16_t  runningStatus, uint16_t freeCaMode) = 0;
    virtual void AddEventDescriptor(EventId eventId, std::string &data) = 0;

    static EitTableInterface * CreateInstance(TableId tableId, ServiceId serviceId, Version versionNumber,
        TsId transportStreamId, NetId originalNetworkId);
};

/**********************class NitTableInterface**********************/
class NitTableInterface: public SiTableInterface
{
public:
    NitTableInterface(){};
    virtual ~NitTableInterface(){};
    
    virtual void AddDescriptor(std::string &data) = 0;
    virtual void AddTs(TsId tsId, OnId onId) = 0;
    virtual void AddTsDescriptor(TsId tsId, std::string &data) = 0; 
    
    static NitTableInterface * CreateInstance(TableId tableId, NetId networkId,  Version versionNumber);
};

/**********************class SdtTableInterface**********************/
class SdtTableInterface: public SiTableInterface
{
public:
    SdtTableInterface(){};
    virtual ~SdtTableInterface(){};
    
    virtual void AddService(ServiceId serviceId, uchar_t eitScheduleFlag, 
                    uchar_t eitPresentFollowingFlag, uint16_t runningStatus, uint16_t freeCaMode) = 0;
    virtual void AddServiceDescriptor(ServiceId serviceId, std::string &data) = 0;
    
    static SdtTableInterface * CreateInstance(TableId tableId, TsId transportStreamId, Version versionNumber, NetId originalNetworkId);
};

#endif