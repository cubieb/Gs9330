#ifndef _Types_h_
#define _Types_h_

#define MaxNitSectionLength     1024

#define SectionSyntaxIndicator  0x1
#define Reserved1Bit            0x1
#define Reserved2Bit            0x3
#define Reserved4Bit            0xF

#define GenerateInlineCodes(Type) \
inline bool operator==(const Type& left, const Type& right) \
{ \
    return (left.Compare(right) == 0); \
} \
inline bool operator!=(const Type& left, const Type& right) \
{ \
    return (left.Compare(right) != 0); \
} \
inline bool operator>(const Type& left, const Type& right) \
{ \
    return (left.Compare(right) > 0); \
} \
inline bool operator<(const Type& left, const Type& right) \
{ \
    return (left.Compare(right) < 0); \
} \
inline std::ostream& operator << (std::ostream& os, const Type& value) \
{ \
    value.Put(os); \
    return os; \
}

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