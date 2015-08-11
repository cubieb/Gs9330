#include "SystemInclude.h"
#include "Common.h"

#include "Descriptor.h"

using namespace std;
/**********************class Discriptor**********************/
int Discriptor::Compare(const Discriptor& right) const
{
    size_t leftSize = GetCodesSize();
    size_t rightSize = right.GetCodesSize();

    shared_ptr<uchar_t> leftBuf(new uchar_t[leftSize], UcharDeleter());
    shared_ptr<uchar_t> rightBuf(new uchar_t[rightSize], UcharDeleter());
    int ret = memcmp(leftBuf.get(), rightBuf.get(), min(leftSize, rightSize));
    if (ret != 0)
        return ret;

    return (leftSize >  rightSize ? 1 : 0);
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


