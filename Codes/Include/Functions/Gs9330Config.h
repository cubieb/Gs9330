#ifndef _Gs9330Config_h_
#define _Gs9330Config_h_

#if defined(_WIN32)
struct Config
{
    std::string xmlDir;
    std::string regionCode;
    uint32_t    tsDstIp;
    uint16_t    tsDstPort;
    uint32_t    tsInterval;  /* upd packet interval in mini seconds */
};

class ConfigDataWrapper
{
public:
    ConfigDataWrapper();
    void ReadConfig(Config&);
};
#endif

#endif