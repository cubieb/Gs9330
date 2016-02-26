#ifndef _NetworkRelationWrapperInterface_h_
#define _NetworkRelationWrapperInterface_h_

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

/**********************class NetworkCfgWrapperInterface**********************/
/* Example:
    NetworksInterface *networks = CreateNetworksInterface();
    NetworkCfgWrapperInterface<NetworksInterface> networkCfgWrapper;
    networkCfgWrapper.Select(*networks, "D:\\Project.VS\\Gs9330.Ver.2.0\\VcProject\\receiver.xml");

    cout << *networks;
*/
template<typename Networks>
class NetworkRelationWrapperInterface
{
public:
    NetworkRelationWrapperInterface() {};
    virtual ~NetworkRelationWrapperInterface() {};

    std::error_code Select(Networks &networks, const char *xmlPath)
    {
        if ((ACE_OS::access(xmlPath, F_OK)) != 0)
        {
            return  make_error_code(std::errc::no_such_file_or_directory);
        }

        Networks::iterator iter;
        for (iter = networks.Begin(); iter != networks.End(); ++iter)
        {
            (*iter)->SetParentNetId(0);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        if (doc == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }
    
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        //xpathExpr cant be "/Root/NetNode[*], because Network has no child node."
        xmlChar *xpathExpr = (xmlChar*)"/Root/NetNode";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;

        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            xmlNodePtr node = nodes->nodeTab[i];
            NetId netId = GetXmlAttrValue<NetId>(node, (const xmlChar*)"NetID");
            NetId parentNetId = GetXmlAttrValue<NetId>(node, (const xmlChar*)"FatherID");
            iter = networks.Find(netId);
            if (iter != networks.End())
            {
                (*iter)->SetParentNetId(parentNetId);
            }
        }

        xmlCleanupParser();
        return std::error_code();
    }
};

#pragma warning(pop)
#endif