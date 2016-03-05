#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Configuration */
#include "DirCfg.h"

DirCfgInterface * DirCfgInterface::CreateInstance(const char *xmlPath)
{
    return new DirCfg(xmlPath);
}

/**********************class DirCfg**********************/
/* public function */
DirCfg::DirCfg(const char *xmlPath)
    : xmlDir(xmlPath)
{
}

DirCfg::~DirCfg()
{
}

const char * DirCfg::GetXmlDir()
{
    return xmlDir.c_str();
}
