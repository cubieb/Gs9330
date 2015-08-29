#ifndef _DataWrapper_h_
#define _DataWrapper_h_

class Section;

/**********************class DataWrapper**********************/
class DataWrapper
{
public:
    typedef std::function<void(Section&, uint16_t)> DbInsertHandler;;

    DataWrapper(DbInsertHandler& handler)
        : handler(handler)
    {}

    virtual ~DataWrapper() 
    {}
    virtual void Start() = 0;

    void HandleDbInsert(Section& section, uint16_t sectionSn)
    {
        handler(section, sectionSn);
    }

private:
    DbInsertHandler handler;
};

#endif