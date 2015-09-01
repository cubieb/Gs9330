#ifndef _DataWrapper_h_
#define _DataWrapper_h_

class Section;

/**********************class DataWrapper**********************/
class DataWrapper
{
public:
    typedef std::function<void(uint16_t netId, std::shared_ptr<Section> section, uint16_t sectionSn)> DbInsertHandler;;

    DataWrapper(DbInsertHandler& handler)
        : handler(handler)
    {}

    virtual ~DataWrapper() 
    {}
    virtual void Start() = 0;

    void HandleDbInsert(uint16_t netId, std::shared_ptr<Section> section, uint16_t sectionSn)
    {
        handler(netId, section, sectionSn);
    }

private:
    DbInsertHandler handler;
};

#endif