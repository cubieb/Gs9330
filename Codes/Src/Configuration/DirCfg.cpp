#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Configuration */
#include "DirCfg.h"

DirCfgInterface * DirCfgInterface::CreateInstance()
{
    return new DirCfg;
}

/**********************class DirCfg**********************/
/* public function */
DirCfg::DirCfg()
{
}

DirCfg::~DirCfg()
{
}

const char * DirCfg::GetXmlDir()
{
    return xmlDir.c_str();
}

void DirCfg::SetXmlDir(const char *xmlDir)
{
    this->xmlDir = xmlDir;
}