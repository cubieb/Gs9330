#ifndef _NetworkRelationWrapperInterface_h_
#define _NetworkRelationWrapperInterface_h_

#include "Include/Foundation/SystemInclude.h"

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

    void Select(Networks &networks, const char *xmlPath)
    {
        Networks::iterator iter;
        for (iter = networks.Begin(); iter != networks.End(); ++iter)
        {
            (*iter)->SetParentNetId(0);
        }

        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        assert(doc != nullptr);
    
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        assert(xpathCtx != nullptr);

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
    }
};

#endif