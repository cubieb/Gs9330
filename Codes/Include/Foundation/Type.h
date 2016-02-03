#ifndef _Type_h_
#define _Type_h_

typedef uint16_t BouquetId;
typedef uint16_t EventId;
typedef uint16_t NetId;
typedef uint16_t OnId;
typedef uint16_t Pid;
typedef uchar_t  SectionNumber;
typedef uint16_t ServiceId;
typedef uchar_t  TableId;
typedef uint16_t TsId;  //Transport Stream Id
typedef uchar_t  Version;
typedef uint32_t SiTableKey;

#define TsPacketSize 188

#define MaxBatSectionLength     1024
#define MaxNitSectionLength     1024
#define MaxSdtSectionLength     1024
#define MaxEitSectionLength     4096

#define NitSectionSyntaxIndicator  0x1
#define SdtSectionSyntaxIndicator  0x1
#define BatSectionSyntaxIndicator  0x1
#define EitSectionSyntaxIndicator  0x1

#define Reserved1Bit               0x1
#define Reserved2Bit               0x3
#define Reserved4Bit               0xF
#define Reserved6Bit               0x3F
#define Reserved8Bit               0xFF
#define Reserved12Bit              0xFFF

#define BatPid                     0x0011
#define EitPid                     0x0012
#define NitPid                     0x0010
#define SdtPid                     0x0011

#define BatTableId                 0x4A
#define EitActualPfTableId         0x4E
#define EitOtherPfTableId          0x4F
#define EitActualSchTableId        0x50
#define EitOtherSchTableId         0x60
#define NitActualTableId           0x40
#define NitOtherTableId            0x41
#define SdtActualTableId           0x42
#define SdtOtherTableId            0x46

#define UdpPayloadSize (188*7)

/*
 * From boost documentation:
 * The following piece of macro magic joins the two 
 * arguments together, even when one of the arguments is
 * itself a macro (see 16.3.1 in C++ standard).  The key
 * is that macro expansion of macro arguments does not
 * occur in JoinName2 but does in JoinName.
 */
#define JoinName(symbol1, symbol2)  JoinName1(symbol1, symbol2)
#define JoinName1(symbol1, symbol2) JoinName2(symbol1, symbol2)
#define JoinName2(symbol1, symbol2) symbol1##symbol2

#endif