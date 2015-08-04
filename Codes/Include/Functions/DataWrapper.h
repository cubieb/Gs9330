#ifndef _DataWrapper_h_
#define _DataWrapper_h_

/*
uimsbf:  unsigned integer most significant bit first
bslbf :  bit string, left bit first

network_information_section()
{
    table_id                         8 uimsbf
    section_syntax_indicator         1 bslbf
    reserved_future_use              1 bslbf
    reserved                         2 bslbf
    section_length                   12 uimsbf
    network_id                       16 uimsbf
    reserved                         2 bslbf
    version_number                   5 uimsbf
    current_next_indicator           1 bslbf
    section_number                   8 uimsbf
    last_section_number              8 uimsbf
    reserved_future_use              4 bslbf
    network_descriptors_length       12 uimsbf
    for(i=0;i<N;i++)
    {
        descriptor()
    }
    reserved_future_use              4 bslbf
    transport_stream_loop_length     12 uimsbf
    for(i=0;i<N;i++)
    {
        transport_stream_id          16 uimsbf
        original_network_id          16 uimsbf
        reserved_future_use          4 bslbf
        transport_descriptors_length 12 uimsbf
        for(j=0;j<N;j++)
        {
            //cable_delivery_system_descriptor() must be included.
            descriptor()  
        }
    }
    CRC_32                           32 rpchof
}
*/
template<typename Nit, typename DescriporFactory>
class NitWrapper
{
public:
    typedef NitWrapper<Nit, DescriporFactory> MyType;
    typedef std::function<void(const MyType&)> Trigger;
    NitWrapper(DescriporFactory& theFactory, Trigger& theTrigger)
        : factory(theFactory), trigger(theTrigger)
    {}
    
    virtual ~NitWrapper() 
    {}

    virtual std::error_code FillNit(Nit& nit) const = 0;

private:
    DescriporFactory factory;
    Trigger trigger;
};

#endif