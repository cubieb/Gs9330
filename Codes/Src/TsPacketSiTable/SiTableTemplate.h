#ifndef _SiTableTemplate_h_
#define _SiTableTemplate_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"

#include "CatchHelper.h"

/**********************class VarHelper**********************/
class VarHelper
{
public:
    VarHelper() {};
    ~VarHelper() {};

    size_t GetCodesSize() const 
    { 
        return 0;
    }

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const
    {
        return 0;
    }
};

/**********************class SiTableTemplate**********************/
template <typename Var1, typename Var2, size_t FixedSize, size_t VarSize>
class SiTableTemplate: public SiTableInterface
{
public:
    virtual size_t GetCodesSize(TableId tableId, TsId tsId, SectionNumber secIndex) const
    {
        SectionNumber secNumber = (SectionNumber)GetSecNumber(tableId, tsId);
        if (secNumber == 0)
            return 0;

#ifdef UseCatchOptimization
        CatchId catchId = catchIdHelper.GetCatchId(tableId, tsId, secIndex);
        map<CatchId, size_t>::iterator catchIter = codeSizeCatches.find(catchId);
        if (catchIter != codeSizeCatches.end())
        {
            return catchIter->second;
        }
#endif

        //we assume all descriptor to be packed in first section.
        assert(var1.GetCodesSize() <= VarSize);
        //check secIndex is valid.
        assert(secIndex < secNumber);

        size_t var1Size = var1.GetCodesSize();
        size_t maxSize = VarSize - var1Size;
        size_t offset = 0;

        for (SectionNumber i = 0; i < secIndex; ++i)
        {
            offset = offset + var2.GetCodesSize(tableId, maxSize, offset);
            var1Size = 0;
            maxSize = VarSize;
        }

        size_t var2Size = var2.GetCodesSize(tableId, maxSize, offset);
        size_t size = FixedSize + var1Size + var2Size;

#ifdef UseCatchOptimization
        codeSizeCatches.insert(make_pair(catchId, size));
#endif
        return size;
    }

    virtual uint_t GetSecNumber(TableId tableId, TsId tsId) const
    {        
        if (!CheckTableId(tableId) || !CheckTsId(tsId))
            return 0;

#ifdef UseCatchOptimization
        CatchId catchId = catchIdHelper.GetCatchId(tableId, tsId);
        map<CatchId, uint_t>::iterator catchIter = secNumberCatches.find(catchId);
        if (catchIter != secNumberCatches.end())
        {
            return catchIter->second;
        }
#endif

        uint_t secNumber = 1;
        size_t maxSize = VarSize - var1.GetCodesSize();
        size_t offset = 0;
        offset = var2.GetCodesSize(tableId, maxSize, offset);
    
        size_t size;
        maxSize = VarSize;
        while ((size = var2.GetCodesSize(tableId, maxSize, offset)) != 0)
        {
            offset = offset + size;
            ++secNumber;
        }

#ifdef UseCatchOptimization
        secNumberCatches.insert(make_pair(catchId, secNumber));
#endif
        return secNumber;
    }

    virtual size_t MakeCodes(TableId tableId, TsId tsId, 
                     uchar_t *buffer, size_t bufferSize,
                     SectionNumber secIndex) const
    {
        SectionNumber secNumber = (SectionNumber)GetSecNumber(tableId, tsId);
        if (secNumber == 0)
            return 0;

        size_t size = GetCodesSize(tableId, tsId, secIndex);
        assert(size <= bufferSize && size != 0);

#ifdef UseCatchOptimization
        CatchId catchId = catchIdHelper.GetCatchId(tableId, tsId, secIndex);
        map<CatchId, uchar_t*>::iterator catchIter = codeCatches.find(catchId);
        if (catchIter != codeCatches.end())
        {
            memcpy(buffer, catchIter->second, size);
            return size;
        }
#endif

        //we assume all descriptor to be packed in first section.
        assert(var1.GetCodesSize() <= VarSize);
        //check secIndex is valid.
        assert(secIndex < secNumber);
                
        size_t var1Size = var1.GetCodesSize();
        size_t maxSize = VarSize - var1Size;
        size_t offset = 0;
    
        /* calculate offset */
        for (SectionNumber i = 0; i < secIndex; ++i)
        {
            offset = offset + var2.GetCodesSize(tableId, maxSize, offset);
            var1Size = 0;
            maxSize = VarSize;
        }

        uchar_t *ptr = buffer;
        WriteHelper<uint16_t> writeHelper(ptr + sizeof(TableId), ptr + sizeof(TableId) + sizeof(TableSize));
        ptr = ptr + MakeCodes1(tableId, ptr, buffer + bufferSize - ptr, var1Size, 
                               secIndex, secNumber - 1);        
        ptr = ptr + MakeCodes2(tableId, ptr, buffer + bufferSize - ptr, maxSize, offset); 
        writeHelper.Write((SectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12), ptr + 4); 
        ptr = ptr + Write32(ptr, Crc32::CalculateCrc(buffer, ptr - buffer));

        
#ifdef UseCatchOptimization
        uchar_t *codeCatch = new uchar_t[size];
        memcpy(codeCatch, buffer, size);

        codeCatches.insert(make_pair(catchId, codeCatch));
#endif
        assert(size == ptr - buffer);
        return ptr - buffer;
    }

protected:
    void ClearCatch()
    {
#ifdef UseCatchOptimization
        codeSizeCatches.clear();
        map<CatchId, uchar_t*>::iterator iter;
        for (iter = codeCatches.begin(); iter != codeCatches.end(); ++iter)
        {
            delete[] iter->second;
        }
        codeCatches.clear();
        secNumberCatches.clear();
#endif
    }

    virtual bool CheckTableId(TableId tableId) const = 0;
    virtual bool CheckTsId(TsId tsid) const = 0;
    virtual size_t MakeCodes1(TableId tableId, uchar_t *buffer, size_t bufferSize, size_t var1Size,
                              SectionNumber secNumber, SectionNumber lastSecNumber) const = 0;    
    virtual size_t MakeCodes2(TableId tableId, uchar_t *buffer, size_t bufferSize,
                              size_t var2MaxSize, size_t var2Offset) const = 0;    
protected:
    Var1 var1;
    Var2 var2;

private:
#ifdef UseCatchOptimization
    mutable CatchIdHelper catchIdHelper;
    mutable std::map<CatchId, size_t> codeSizeCatches;
    mutable std::map<CatchId, uchar_t*> codeCatches;
    mutable std::map<CatchId, uint_t> secNumberCatches;
#endif
};

#endif