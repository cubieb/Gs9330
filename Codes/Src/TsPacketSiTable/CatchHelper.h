#ifndef _Bat_h_
#define _Bat_h_

/**********************class CatchIdHelper**********************/
class CatchIdHelper
{
public:
    CatchIdHelper();
    CatchId GetCatchId(TableId tableId, const TsIds &tsIds);
    CatchId GetCatchId(TableId tableId, const TsIds &tsIds, SectionNumber secIndex);

private:
    uint_t index;
    std::map<TableId, std::map<TsIds, CatchId>> catchIds;
};

#endif