#ifndef _Types_h_
#define _Types_h_

#define MaxNitSectionLength     1024
#define MaxSdtSectionLength     1024

#define NitSectionSyntaxIndicator  0x1
#define SdtSectionSyntaxIndicator  0x1

#define Reserved1Bit               0x1
#define Reserved2Bit               0x3
#define Reserved4Bit               0xF
#define Reserved6Bit               0x3F
#define Reserved8Bit               0xFF
#define Reserved12Bit              0xFFF

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