#ifndef _DirCfgWrapperInterface_h_
#define _DirCfgWrapperInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/Deleter.h"
#include "Include/Foundation/XmlHelper.h"

template<typename DirCfg>
class DirCfgWrapperInterface
{
public:
    DirCfgWrapperInterface() {};
    virtual ~DirCfgWrapperInterface() {};

    void Select(DirCfg &dirCfg, const char *xmlPath)
    {
        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        assert(doc != nullptr);

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        node = xmlFirstElementChild(node);  /* ChildXML node */
        assert(strcmp((const char *)node->name, "ChildXML") == 0);

        shared_ptr<xmlChar> cfgDir = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"CfgDir");
        dirCfg.SetCfgDir((char*)cfgDir.get());

        shared_ptr<xmlChar> xmlDir = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"ReceivDir");
        dirCfg.SetXmlDir((char*)xmlDir.get());
    }
};

#endif