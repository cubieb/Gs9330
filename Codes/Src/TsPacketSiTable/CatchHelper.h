#ifndef _CatchIdHelper_h_
#define _CatchIdHelper_h_

/**********************class CatchIdHelper**********************/
class CatchIdHelper
{
public:
    static CatchId GetCatchId(TableId tableId, TsId tsId);
    static CatchId GetCatchId(TableId tableId, TsId tsId, SectionNumber secIndex);
};

#endif