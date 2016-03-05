#ifndef _DirCfg_h_
#define _DirCfg_h_

#include "Include/Configuration/DirCfgInterface.h"

class DirCfg: public DirCfgInterface
{
public:
    DirCfg(const char *xmlPath);
    virtual ~DirCfg();
        
    const char * GetXmlDir();

private:
    std::string xmlDir;
};

#endif