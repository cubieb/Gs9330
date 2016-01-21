#ifndef _NetworkCfgWrapperInterface_h_
#define _NetworkCfgWrapperInterface_h_

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
class NetworkCfgWrapperInterface
{
public:
    NetworkCfgWrapperInterface() {};
    virtual ~NetworkCfgWrapperInterface() {};

    void Select(Networks &networks, const char *xmlPath)
    {
        shared_ptr<xmlDoc> doc(xmlParseFile(xmlPath), XmlDocDeleter());
        assert(doc != nullptr);

        xmlNodePtr node = xmlDocGetRootElement(doc.get());
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        assert(xpathCtx != nullptr);

        xmlChar *xpathExpr = (xmlChar*)"/root/network[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        assert(nodes != nullptr);

        uint_t recieverId = 0;
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            NetId netId = GetXmlAttrValue<NetId>(node, (const xmlChar*)"id");

            NetworkCfgInterface *network = CreateNetworkCfgInterface(netId);
            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                SharedXmlChar ip = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"ip");

                struct sockaddr_in socketAddr;
                socketAddr.sin_family = AF_INET;
                socketAddr.sin_addr.s_addr = inet_addr((char *)ip.get());
                socketAddr.sin_port = htons(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"port"));
                ReceiverInterface *receiver = CreateReceiverInterface(recieverId++, socketAddr);

                xmlNodePtr tsIdNode;
                for (tsIdNode = xmlFirstElementChild(node); 
                     tsIdNode != nullptr; 
                     tsIdNode = xmlNextElementSibling(tsIdNode))
                {
                    TsId tsId = GetXmlContent<TsId>(tsIdNode);
                    receiver->Add(tsId);
                }

                network->Add(receiver);
            }

            networks.Add(network);
        }
    }
};

#endif