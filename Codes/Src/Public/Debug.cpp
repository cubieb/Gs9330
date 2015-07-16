#include "SystemInclude.h"

#include "Debug.h"

using namespace std;
map<string, bool> DebugFlag::flags;

DebugFlag::DebugFlag()
{
}

bool DebugFlag::GetState(string const& funcName)
{
    map<string, bool>::iterator iter = flags.find(funcName);
    if (iter != flags.end())
    {
        return iter->second;
    }

    return false;
}

void DebugFlag::SetState(string const& funcName, bool isDebug)
{
    map<string, bool>::iterator iter = flags.find(funcName);
    if (iter == flags.end())
    {
        flags.insert(make_pair(funcName, isDebug));
        return;
    }

    iter->second = isDebug;
}

ostream& DgbClearStream(char const* funcName, uint32_t line)
{
    static ostream nullStream(0);
    static bool first = true;
    if (first)
    {
        first = false;
        nullStream.clear(ios::eofbit);
    }

    DebugFlag flag;
    ostream& os = flag.GetState(funcName) ? cout : nullStream;

    return os;
}

ostream& DgbOStream(char const* funcName, uint32_t line)
{
    ostream& os = DgbClearStream(funcName, line);

    os << funcName << ", " << line << "> ";
    return os;
}

ostream& ErrOStream(char const* funcName, uint32_t line)
{
    cerr << funcName << ", " << line << "> ";
    return cerr;
}

