#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"

#include "CatchHelper.h"
using namespace std;

/**********************class CatchIdHelper**********************/
CatchId CatchIdHelper::GetCatchId(TableId tableId, TsId tsId)
{    
    return GetCatchId(tableId, tsId, ReservedSectionNumber);
}

CatchId CatchIdHelper::GetCatchId(TableId tableId, TsId tsId, SectionNumber secIndex)
{
    return ((tableId & MaxTableId)<< (TsIdBits + SectionNumberBits)) 
         | ((tsId  & MaxTsId) << SectionNumberBits)
         | (secIndex & MaxSectionNumber);
}