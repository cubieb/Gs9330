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

    return (leftSize >  rightSize ? 1 : -1);
}

