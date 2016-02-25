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

CatchId CatchIdHelper::GetCatchId(TableId tableId, TsId tsId)
{
    map<TableId, map<TsId, CatchId>>::iterator tableIdIter;
    tableIdIter = catchIds.find(tableId);
    if (tableIdIter == catchIds.end())
    {
        CatchId id = ((CatchId)tableId << 24) | (CatchId)index;
        ++index;
        map<TsId, CatchId> tsIdsToCatchId;
        tsIdsToCatchId.insert(make_pair(tsId, id));
        catchIds.insert(make_pair(tableId, tsIdsToCatchId));

        return id;
    }

    map<TsId, CatchId> &tsIdsToCatchId = tableIdIter->second;
    map<TsId, CatchId>::iterator tsIdsIter = tsIdsToCatchId.find(tsId);
    if (tsIdsIter == tsIdsToCatchId.end())
    {
        CatchId id = ((CatchId)tableId << 24) | (CatchId)index;
        ++index;
        tsIdsToCatchId.insert(make_pair(tsId, id));
        return id;
    }
        
    return tsIdsIter->second;
}

CatchId CatchIdHelper::GetCatchId(TableId tableId, TsId tsId, SectionNumber secIndex)
{
    return GetCatchId(tableId, tsId) | ((CatchId)secIndex << 16);
}