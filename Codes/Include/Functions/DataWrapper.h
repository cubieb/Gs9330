#ifndef _DataWrapper_h_
#define _DataWrapper_h_

template<typename Nit>
class NitWrapper
{
public:
    typedef NitWrapper<Nit> MyType;
    typedef std::function<void(const MyType&)> Trigger;

    NitWrapper(Trigger& theTrigger)
        : trigger(theTrigger)
    {}

    virtual ~NitWrapper() 
    {}
    virtual void Start() const = 0;
    virtual std::error_code FillNit(Nit& nit) const = 0;

protected:
    Trigger trigger;
};

#endif