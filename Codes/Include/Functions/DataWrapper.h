#ifndef _DataWrapper_h_
#define _DataWrapper_h_

/**********************class DataWrapper**********************/
template<typename Table>
class DataWrapper
{
public:
    typedef DataWrapper<Table> MyType;
    typedef std::function<void(const MyType&)> Trigger;

    DataWrapper(Trigger& theTrigger)
        : trigger(theTrigger)
    {}

    virtual ~DataWrapper() 
    {}
    virtual void Start() const = 0;
    virtual std::error_code Fill(Table&) const = 0;

protected:
    Trigger trigger;
};

#endif