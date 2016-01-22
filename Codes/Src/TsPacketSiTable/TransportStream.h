#ifndef _TransportStream_h_
#define _TransportStream_h_

#include "Descriptor.h"       //Descriptor 

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
    
    size_t GetCodesSize(const std::list<TsId>& tsIds) const;
    size_t MakeCodes(const std::list<TsId>& tsIds, uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    //void Put(std::ostream& os) const;
private:
    std::list<TransportStream*> transportStreams;
};

#endif