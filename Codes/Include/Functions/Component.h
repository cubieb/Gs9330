#ifndef _SectionBase_h_
#define _SectionBase_h_

#include "Types.h"
/**********************class Component**********************/
class Component
{
public:
    Component() {}
    virtual ~Component() {}

    virtual size_t GetCodesSize() const = 0;
    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const = 0;

    /* the following function is provided just for debug */
    int Compare(const Component& right) const;
    virtual void Put(std::ostream& os) const = 0;
};
GenerateInlineCodes(Component);

/**********************class Descriptor**********************/
class Descriptor: public Component
{
public:    
    Descriptor() {}
    virtual ~Descriptor() {}

    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const = 0;
    virtual size_t GetCodesSize() const = 0;

    /* the following function is provided just for debug */
    virtual void Put(std::ostream& os) const = 0;
};

/**********************class Section**********************/
class Section: public Component
{
public:
    Section() {}
    virtual ~Section() {}

    virtual uint16_t GetPid()  const = 0; 
    
    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const = 0;
    virtual size_t GetCodesSize() const = 0;

    /* the following function is provided just for debug */
    virtual void Put(std::ostream& os) const = 0;
};

#endif