#ifndef _SiTableInterface_h_
#define _SiTableInterface_h_

/**********************class SiTableInterface**********************/
class SiTableInterface
{
public: 
    SiTableInterface(){};
    virtual ~SiTableInterface(){};
    
    virtual void AddDescriptor(std::string &data) 
    { assert(false);}
    virtual void AddEvent(EventId eventId, const char *startTime, time_t duration,
                          uint16_t  runningStatus, uint16_t freeCaMode)
    { assert(false);}
    virtual void AddEventDescriptor(EventId eventId, std::string &data)
    { assert(false);}
    virtual void AddService(ServiceId serviceId, uchar_t eitScheduleFlag, 
                            uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
                            uint16_t freeCaMode)
    { assert(false);}
    virtual void AddServiceDescriptor(ServiceId serviceId, std::string &data)
    { assert(false);}
    virtual void AddTs(TsId tsId, OnId onId) 
    { assert(false);}
    virtual void AddTsDescriptor(TsId tsId, std::string &data) 
    { assert(false);}

    virtual size_t GetCodesSize(TableId tableId, TsId tsId, 
                                SectionNumber secIndex) const = 0;
    //nit: network_id, sdt: transport_stream_id, bat: bouquet_id, eit: transport_stream_id+service_id
    virtual SiTableKey GetKey() const = 0;  
    virtual uint_t GetSecNumber(TableId tableId, TsId tsId) const = 0;
    //table type: nit 0x41, 0x41; sdt 0x42, 0x46;  bat 0x4a; eit 0x4e, 0x4f, 0x50, 0x60
    virtual TableId GetTableId() const = 0;  
    virtual size_t MakeCodes(TableId tableId, TsId tsId, 
		                     uchar_t *buffer, size_t bufferSize, 
                             SectionNumber secIndex) const = 0;
    virtual void RefreshCatch() {};

    /* static function */
    static SiTableInterface * CreateBatInstance(TableId tableId, BouquetId bouquetId,  Version versionNumber);
    static SiTableInterface * CreateEitInstance(TableId tableId, ServiceId serviceId, Version versionNumber,
                                                TsId transportStreamId, NetId originalNetworkId);
    static SiTableInterface * CreateNitInstance(TableId tableId, NetId networkId,  Version versionNumber);
    static SiTableInterface * CreateSdtInstance(TableId tableId, TsId transportStreamId, 
                                                Version versionNumber, NetId originalNetworkId);
};

class CompareSiTableIdAndKey: public std::unary_function<SiTableInterface, bool>
{
public:
    CompareSiTableIdAndKey(TableId tableId, SiTableKey key)
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
    SiTableKey key;
};

#endif