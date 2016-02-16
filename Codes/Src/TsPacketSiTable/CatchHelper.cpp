#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"

#include "CatchHelper.h"
using namespace std;

/**********************class CatchIdHelper**********************/
CatchIdHelper::CatchIdHelper()
{
    index = 0;
}

CatchId CatchIdHelper::GetCatchId(TableId tableId, const TsIds &tsIds)
{
    map<TableId, map<TsIds, CatchId>>::iterator tableIdIter;
    tableIdIter = catchIds.find(tableId);
    if (tableIdIter == catchIds.end())
    {
        CatchId id = ((CatchId)tableId << 56) | (CatchId)index;
        ++index;
        map<TsIds, CatchId> tsIdsToCatchId;
        tsIdsToCatchId.insert(make_pair(tsIds, id));
        catchIds.insert(make_pair(tableId, tsIdsToCatchId));

        return id;
    }

    map<TsIds, CatchId> &tsIdsToCatchId = tableIdIter->second;
    map<TsIds, CatchId>::iterator tsIdsIter = tsIdsToCatchId.find(tsIds);
    if (tsIdsIter == tsIdsToCatchId.end())
    {
        CatchId id = ((CatchId)tableId << 56) | (CatchId)index;
        ++index;
        tsIdsToCatchId.insert(make_pair(tsIds, id));
        return id;
    }
        
    return tsIdsIter->second;
}

CatchId CatchIdHelper::GetCatchId(TableId tableId, const TsIds &tsIds, SectionNumber secIndex)
{
    return GetCatchId(tableId, tsIds) | ((CatchId)secIndex << 48);
}