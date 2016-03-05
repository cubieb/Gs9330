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
 
    virtual const char * GetXmlDir() = 0;

    static DirCfgInterface * CreateInstance(const char *xmlPath);
};

#endif