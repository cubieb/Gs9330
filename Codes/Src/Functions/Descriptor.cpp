#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Types.h"
#include "Descriptor.h"

using namespace std;
/**********************class Descriptor**********************/
int Descriptor::Compare(const Descriptor& right) const
{
    size_t leftSize = GetCodesSize();
    size_t rightSize = right.GetCodesSize();

    shared_ptr<uchar_t> leftBuf(new uchar_t[leftSize], UcharDeleter());
    shared_ptr<uchar_t> rightBuf(new uchar_t[rightSize], UcharDeleter());
    MakeCodes(leftBuf.get(), leftSize);
    right.MakeCodes(rightBuf.get(), rightSize);

    int ret = memcmp(leftBuf.get(), rightBuf.get(), min(leftSize, rightSize));
    if (ret == 0)
        return ret;

    return (leftSize >  rightSize ? 1 : -1);
}

/**********************class UcharDescriptor**********************/
size_t UcharDescriptor::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    assert(dataSize <= bufferSize);
    memcpy(buffer, data.get(), dataSize);
    return dataSize;
}

size_t UcharDescriptor::GetCodesSize() const
{
    return dataSize;
}

void UcharDescriptor::Put(std::ostream& os) const
{
    os << (char*)data.get();
}

/**********************class Descriptors**********************/
void Descriptors::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    DescriporFactory factory;
    shared_ptr<Descriptor> discripter(factory.Create(tag, data, dataSize));
    if (discripter != nullptr)
    {
        descriptors.push_back(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)tag << endl;
}

size_t Descriptors::GetCodesSize() const
{
    size_t size = 0;
    for (const auto iter: descriptors)
    {
        size = size + iter->GetCodesSize();
    }

    /* 2 bytes for reserved_future_use and transport_descriptors_length */
    return (size + 2); 
}

size_t Descriptors::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;    
    assert(Descriptors::GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, 0);
    size_t size = 0;
    for (auto iter = descriptors.begin(); iter != descriptors.end(); ++iter)
    {
        ptr = ptr + (*iter)->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + (*iter)->GetCodesSize();
    }
    uint16_t ui16Value = (Reserved4Bit << 12) | size;
    Write16(buffer, ui16Value);

    return (ptr - buffer);
}

int Descriptors::Compare(const Descriptors& right) const
{
    size_t leftSize = GetCodesSize();
    size_t rightSize = right.GetCodesSize();

    shared_ptr<uchar_t> leftBuf(new uchar_t[leftSize], UcharDeleter());
    shared_ptr<uchar_t> rightBuf(new uchar_t[rightSize], UcharDeleter());
    MakeCodes(leftBuf.get(), leftSize);
    right.MakeCodes(rightBuf.get(), rightSize);

    int ret = memcmp(leftBuf.get(), rightBuf.get(), min(leftSize, rightSize));
    if (ret == 0)
        return ret;

    return (leftSize >  rightSize ? 1 : -1);
}

void Descriptors::Put(std::ostream& os) const
{
    uint_t i = 0;
    for(auto iter: descriptors)
    {
        os << "Destripter " << i++ << ": " << *iter << endl;
    }
}

