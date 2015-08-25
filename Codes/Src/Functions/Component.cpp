#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Component.h"

using namespace std;
/**********************class Component**********************/
int Component::Compare(const Component& right) const
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

    return (leftSize > rightSize ? 1 : -1);
}


/**********************class Components**********************/
void Components::AddComponent(const std::shared_ptr<Component>& component)
{
    components.push_back(component);
}

size_t Components::GetCodesSize() const
{
    /* 2 bytes for reserved_future_use and transport_descriptors_length */
    size_t size = 2;
    for (const auto iter: components)
    {
        size = size + iter->GetCodesSize();
    }
        
    return size; 
}

size_t Components::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;    
    
    ptr = ptr + MakeCodes(buffer, bufferSize, Reserved4Bit);

    assert(ptr - buffer == GetCodesSize());
    return (ptr - buffer);
}

//reserved4Bit: when encodes, there are offten "4 reserved bits + descriptors_loop_length"
//followed by component.  We give users a chance to customize the reserved4Bit
size_t Components::MakeCodes(uchar_t *buffer, size_t bufferSize, uchar_t reserved4Bit) const
{
    uchar_t *ptr = buffer;    
    assert(GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, 0);
    size_t size = 0;
    for (const auto iter: components)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + iter->GetCodesSize();
    }
    uint16_t ui16Value = (reserved4Bit << 12) | size;
    Write16(buffer, ui16Value);

    assert(ptr - buffer == GetCodesSize());
    return (ptr - buffer);
}

void Components::Put(std::ostream& os) const
{
    for (const auto iter: components)
    {
        os << *iter;
    }
}