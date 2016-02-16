#ifndef _TransportStream_h_
#define _TransportStream_h_

#include "Descriptor.h"       //Descriptor 

#pragma pack(push, 1)
struct transport_stream
{
    //for(i=0;i<N;i++)
    //{
        uint16_t transport_stream_id:          16; // uimsbf
        uint16_t original_network_id:          16; //uimsbf
        uint16_t reserved_future_use:          4;  //bslbf
        uint16_t transport_descriptors_length: 12; //uimsbf
    //    for(j=0;j<N;j++)
    //    {
    //        //cable_delivery_system_descriptor() must be included.
    //        descriptor()  
    //    }
    //}
};
#pragma pack(pop)
//TransportStream is used by Bat and Nit, the "size" must less than
//MaxBatDesAndTsContentSize(which is 1008) and MaxNitDesAndTsContentSize(which is 1008), 
//or else this TransportStream can not be packed in one single Bat or Nit section.
#define MaxTransportStreamSize 1008

/**********************class TransportStream**********************/
class TransportStream
{
public:
    TransportStream(TsId transportStreamId, OnId originalNetworkId);
    ~TransportStream();

    void AddDescriptor(Descriptor *descriptor);
    size_t GetCodesSize() const;
    OnId GetOnId() const;
    TsId GetTsId() const; 
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;   

private:
    Descriptors descriptors;
    OnId originalNetworkId;
    TsId transportStreamId;
};

class CompareTransportStreamId: public std::unary_function<TransportStream, bool>
{
public:
    CompareTransportStreamId(TsId tsId)
        : tsId(tsId)
    {}

    result_type operator()(const argument_type &ts)
    {
        return (result_type)(ts.GetTsId() == tsId);
    }    

    result_type operator()(const argument_type *ts)
    {
        return this->operator()(*ts);
    }

private:
    TsId  tsId;
};

/**********************class TransportStreams**********************/
class TransportStreams
{
public:
    TransportStreams();
    ~TransportStreams();

    void AddTransportStream(TsId tsId, OnId onId);
    void AddTsDescriptor(TsId tsId, Descriptor *discriptor);
    
    size_t GetCodesSize(const TsIds &tsIds, 
                        size_t maxSize, size_t &offset) const;
    size_t MakeCodes(const TsIds &tsIds, 
                     uchar_t *buffer, size_t bufferSize, 
                     size_t offset) const;

    /* the following function is provided just for debug */
    //void Put(std::ostream& os) const;
private:
    std::list<TransportStream*> transportStreams;
};

#endif