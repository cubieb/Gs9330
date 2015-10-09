#include "SystemInclude.h"
#include "Common.h"

#include "Nit.h"
#include "Eit.h"
#include "Ts.h"

using namespace std;

/**********************class Segment**********************/
Segment::Segment()
{}

void Segment::Init(shared_ptr<Section> section, size_t segmentSize)
{
    size_t size = 1; //1 byte for pointer_field
    size = size + section->GetCodesSize();
    size_t tail = (segmentSize - (size % segmentSize)) % segmentSize;
    size = size + tail; /* size must be times of segmentSize */
    buffer.reset(new uchar_t[size], UcharDeleter());

    uchar_t *ptr = buffer.get();
    ptr = ptr + Write8(ptr, 0x0); //pointer_field

    ptr = ptr + section->MakeCodes(ptr, size - (buffer.get() - ptr));

    segments.clear();
    for (ptr = buffer.get(); ptr < buffer.get() + size; ptr = ptr + segmentSize)
    {
        segments.push_back(ptr);
    }

    ptr = segments.back();
    memset(ptr + segmentSize - tail, 0xff, tail);
}

Segment::iterator Segment::begin()
{
    return segments.begin();
}

Segment::iterator Segment::end()
{
    return segments.end();
}

uint_t Segment::GetSegmentNumber(shared_ptr<Section> section, size_t segmentSize)
{
    size_t size = 1; //1 byte for pointer_field
    size = size + section->GetCodesSize();

    return (size + segmentSize - 1) / segmentSize;
}

/**********************class Ts**********************/
Ts::Ts(uint16_t pid): pid(pid)
{
    transporPacket.transportPriority = 0;
    transporPacket.adaptationFieldControl = 1;
    transporPacket.continuityCounter = 0;
}

void Ts::SetTransportPriority(uint16_t theTransportPriority)
{
    transporPacket.transportPriority = theTransportPriority;
}

void Ts::SetContinuityCounter(uchar_t theContinuityCounter)
{
    transporPacket.continuityCounter = theContinuityCounter;
}

uint16_t Ts::GetPid()
{
    return pid;
}

size_t Ts::GetCodesSize(const std::bitset<256>& tableIds) const
{
    Segment segment;
    size_t segmentSize = TsPacketSize - sizeof(transport_packet);
    uint32_t segmentNumber = 0; 
    
    for (auto iter: sections)
    {
        if (tableIds.test(iter->GetTableId()))
        {
            segmentNumber = segmentNumber + segment.GetSegmentNumber(iter, segmentSize);
        }

        uchar_t tableId = iter->GetTableId();
        if (tableId == 0x50)
            tableId = 0x4e;
        else if (tableId == 0x60)
            tableId = 0x4f;
        else
            continue;

        if (tableIds.test(tableId))
        {
            shared_ptr<Eit> eit = dynamic_pointer_cast<Eit>(iter);
            segmentNumber = segmentNumber + segment.GetSegmentNumber(eit->GetSubPresentSection(), segmentSize);
            segmentNumber = segmentNumber + segment.GetSegmentNumber(eit->GetSubFollwingtSection(), segmentSize);
        }
    }
    
    return (TsPacketSize * segmentNumber);
}

size_t Ts::MakeCodeImpl(Segment& segment, uchar_t *buffer)
{
    size_t segmentSize = TsPacketSize - sizeof(transport_packet);
    uchar_t *ptr = buffer;
    for (auto iter = segment.begin(); iter != segment.end(); ++iter)
    {
        ptr = ptr + Write8(ptr, 0x47);
        /* transport_error_indicator(1 bit), payload_unit_start_indicator(1 bit), 
            transport_priority(1 bit), PID(13 bits)
            transport_error_indicator = 0;
            transport_priority = 0;
            payload_unit_start_indicator = (first byte of current section) ? 1 : 0;
            */
        uint16_t startIndicator = (iter == segment.begin() ? 1 : 0);
        ptr = ptr + Write16(ptr, (startIndicator << 14) | (transporPacket.transportPriority << 13) | pid);
        /* transport_scrambling_control[2] = '00';
		    adaptation_field_control[2] = '01';
		    continuity_counter[4] = 'xxxx';
		*/
        /* refer to "2.4.3.3 Semantic definition of fields in Transport Stream packet layer",
            continuity_counter should be increase by 1 in all case.  
            when send udp packet, we may send duplicate packet two time, in this circumstance, the
            continuity_counter keep same with the oringinal packet.
            for example, the udp sending function may like this:
            ts.MakeCodes(buffer, bufferSize);
            for (ptr = buffer; ptr = ptr < buffer + buffersize; buffer + 188)
            {
                SendUdp(ptr, 188);
                SendUdp(ptr, 188);   //again
            }
        */
        ptr = ptr + Write8(ptr, transporPacket.adaptationFieldControl << 4 | transporPacket.continuityCounter++);
        ptr = ptr + MemCopy(ptr, segmentSize, *iter, segmentSize);        
    }

    return ptr - buffer;
}

size_t Ts::MakeCodes(uchar_t *buffer, size_t bufferSize, const std::bitset<256>& tableIds)
{
    size_t segmentSize = TsPacketSize - sizeof(transport_packet);
    uchar_t *ptr = buffer;

    for (auto iter: sections)
    {
        if (!tableIds.test(iter->GetTableId()))
        {
            continue;
        }
        
        Segment segment;    
        segment.Init(iter, segmentSize);
        ptr = ptr + MakeCodeImpl(segment, ptr);
    }

    for (auto iter: sections)
    {
        if (iter->GetPid() != 0x0012)
            continue;

        Segment segment;  
        shared_ptr<Eit> eit = dynamic_pointer_cast<Eit>(iter);
        if (tableIds.test(eit->GetSubPresentSection()->GetTableId()))
        {              
            segment.Init(eit->GetSubPresentSection(), segmentSize);       
            ptr = ptr + MakeCodeImpl(segment, ptr);
        }  
                
        if (tableIds.test(eit->GetSubFollwingtSection()->GetTableId()))
        {              
            segment.Init(eit->GetSubFollwingtSection(), segmentSize);       
            ptr = ptr + MakeCodeImpl(segment, ptr);
        } 
    }
    
    assert (ptr - buffer == bufferSize);
    return (ptr - buffer);
}

void Ts::PropagateEitSection()
{
    assert(pid == 0x0012);
    
    for (auto iter: sections)
    {
        dynamic_cast<Eit&>(*iter).PropagateSection(); 
    }
}

void Ts::AddSection(std::shared_ptr<Section> section)
{
    assert(pid == section->GetPid());
    sections.push_back(section);
}

void Ts::RemoveSection(const char *key)
{
    sections.remove_if(CompareSectionKey(key));
}

/* the following function is provided just for debug */
void Ts::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );
    os << "Section Nuber: " << sections.size()
        << endl;
    for (auto iter: sections)
    {
        os << "nid = " << (uint32_t)iter->GetNetworkId() 
            << ", pid = " << (uint32_t)iter->GetPid()
            << ", tableid = " << (uint32_t)iter->GetTableId()
            << ", key = " << iter->GetKey()
            << endl;
    }

    cout.flags(flags);
}