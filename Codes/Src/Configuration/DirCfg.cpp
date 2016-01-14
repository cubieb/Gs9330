#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* Configuration */
#include "DirCfg.h"

DirCfgInterface * CreateDirCfgInterface()
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

const char * DirCfg::GetCfgDir()
{
    return cfgDir.c_str();
};

const char * DirCfg::GetXmlDir()
{
    return xmlDir.c_str();
}

void DirCfg::SetCfgDir(const char *cfgDir)
{
    this->cfgDir = cfgDir;
}

void DirCfg::SetXmlDir(const char *xmlDir)
{
    this->xmlDir = xmlDir;
}