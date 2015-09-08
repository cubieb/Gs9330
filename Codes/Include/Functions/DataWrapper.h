#ifndef _DataWrapper_h_
#define _DataWrapper_h_

class Section;

/**********************class DataWrapper**********************/
class DataWrapper
{
public:
    typedef std::function<void(std::shared_ptr<Section> section)> DbInsertHandler;
    typedef std::function<void(const char *tableName, const char *tableKey)> DbDeleteHandler;

    DataWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler)
        : insertHandler(insertHandler), deleteHandler(deleteHandler)
    {}

    virtual ~DataWrapper() 
    {}
    virtual void Start() = 0;

    //DirMonitor -> XmlDataWrapper<Section>::HandleDbInsert() 
    // -> NotifyDbInsert() ->Controller::HandleDbInsert()
    void NotifyDbInsert(std::shared_ptr<Section> section) const
    {
        insertHandler(section);
    }

    //DirMonitor -> XmlDataWrapper<Section>::HandleDbDelete() 
    // -> NotifyDbDelete()->Controller::HandleDbDelete()
    void NotifyDbDelete(const char *tableName, const char *tableKey) const
    {
        deleteHandler(tableName, tableKey);
    }

private:
    DbInsertHandler insertHandler;
    DbDeleteHandler deleteHandler;
};

#endif