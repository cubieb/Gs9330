#ifndef _DirCfgWrapperInterface_h_
#define _DirCfgWrapperInterface_h_

#include "Include/Foundation/SystemInclude.h"

#pragma warning(push)
#pragma warning(disable:702)   //disable warning caused by ACE library.
#pragma warning(disable:4251)  //disable warning caused by ACE library.
#pragma warning(disable:4996)  //disable warning caused by ACE library.
#include "ace/OS.h"

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

    std::error_code Select(DirCfg &dirCfg, const char *xmlPath)
    {
        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            return  make_error_code(std::errc::no_such_file_or_directory);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        node = xmlFirstElementChild(node);  /* ChildXML node */
        assert(strcmp((const char *)node->name, "ChildXML") == 0);

        shared_ptr<xmlChar> xmlDir = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"ReceivDir");
        dirCfg.SetXmlDir((char*)xmlDir.get());

        return std::error_code();
    }
};

#pragma warning(pop)
#endif