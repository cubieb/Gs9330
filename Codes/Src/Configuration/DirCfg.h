#ifndef _DirCfg_h_
#define _DirCfg_h_

#include "Include/Configuration/DirCfgInterface.h"

class DirCfg: public DirCfgInterface
{
public:
    DirCfg();
    virtual ~DirCfg();
        
    const char * GetXmlDir();
    void SetXmlDir(const char *xmlDir);

private:
    std::string cfgDir;
    std::string xmlDir;
};

#endif