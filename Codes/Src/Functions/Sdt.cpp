#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Crc32.h"
#include "Descriptor.h"
#include "XmlDataWrapper.h"
#include "Sdt.h"

using namespace std;
/**********************class Sdt**********************/
Sdt::Sdt()
{}

uint16_t Sdt::GetPid() const
{
    return Pid;
}

void Sdt::SetTableId(uchar_t data)
{
    tableId = data;
}

size_t Sdt::GetCodesSize() const
{
    return 0;
}

size_t Sdt::MakeCodes(uchar_t *buffer, size_t bufferSize)
{
    return 0;
}

void Sdt::Put(std::ostream& os) const
{
    os  << "tableId = " << (uint_t)tableId << endl ;
}