#ifndef _CatchIdHelper_h_
#define _CatchIdHelper_h_

/**********************class CatchIdHelper**********************/
class CatchIdHelper
{
public:
    CatchIdHelper();
    CatchId GetCatchId(TableId tableId, TsId tsId);
    CatchId GetCatchId(TableId tableId, TsId tsId, SectionNumber secIndex);

private:
    uint_t index;
    std::map<TableId, std::map<TsId, CatchId>> catchIds;
};

#endif