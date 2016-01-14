#ifndef _DirCfgInterface_h_
#define _DirCfgInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"

class DirCfgInterface
{
public:
    DirCfgInterface() {};
    virtual ~DirCfgInterface() {};

    virtual const char * GetCfgDir() = 0; 
    virtual const char * GetXmlDir() = 0;
    virtual void SetCfgDir(const char *cfgDir) = 0;
    virtual void SetXmlDir(const char *xmlDir) = 0;
};
DirCfgInterface * CreateDirCfgInterface();

#endif