#ifndef _TsDataWrapper_h_
#define _TsDataWrapper_h_
#include "SystemError.h"

#include "DirMonitor.h"
#include "DataWrapper.h"

/**********************class TsDataWrapper**********************/
template<typename Section>
class TsDataWrapper: public DataWrapper
{
public:
    typedef DataWrapper MyBase;
    typedef TsDataWrapper<Section> MyType;

    TsDataWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler,
        const char *tsFileDir, const char *tsFileRegularExp);
    virtual ~TsDataWrapper();

    void Start();
    void HandleDbInsert(const char *file);
    void HandleDbDelete(const char *file);

    virtual void CreateSection(const char *file) const = 0;

protected:
    std::string tsFileDir;
    std::string tsFileRegularExp;
};

/**********************class NitTsWrapper**********************/
template<typename Section>
class NitTsWrapper: public TsDataWrapper<Section>
{
public:
    typedef TsDataWrapper<Section> MyBase;
    typedef NitTsWrapper<Section> MyType;    

    NitTsWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler, const char *tsFileDir);
    void CreateSection(const char *file) const;
};

/**********************class SdtTsWrapper**********************/
template<typename Section>
class SdtTsWrapper: public TsDataWrapper<Section>
{
public:
    typedef TsDataWrapper<Section> MyBase;
    typedef SdtTsWrapper<Section> MyType;    

    SdtTsWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler, const char *tsFileDir);
    void CreateSection(const char *file) const;
};

/**********************class BatTsWrapper**********************/
template<typename Section>
class BatTsWrapper: public TsDataWrapper<Section>
{
public:
    typedef TsDataWrapper<Section> MyBase;
    typedef BatTsWrapper<Section> MyType;    

    BatTsWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler, const char *tsFileDir);
    void CreateSection(const char *file) const;
};

/**********************class EitTsWrapper**********************/
template<typename Section>
class EitTsWrapper: public TsDataWrapper<Section>
{
public:
    typedef TsDataWrapper<Section> MyBase;
    typedef NitTsWrapper<Section> MyType;    

    EitTsWrapper(DbInsertHandler& insertHandler, DbDeleteHandler& deleteHandler, const char *tsFileDir);
    void CreateSection(const char *file) const;
};

#endif