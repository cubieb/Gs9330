#ifndef _Descriptor_h_
#define _Descriptor_h_

class Discriptor
{
};

class DescriporFactory
{
public:
    template<int Tag, typename Para>
    Discriptor* operator()(Para para) const
    {
        return nullptr;
    }
};

#endif