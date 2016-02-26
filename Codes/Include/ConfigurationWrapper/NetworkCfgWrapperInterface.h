#ifndef _NetworkCfgWrapperInterface_h_
#define _NetworkCfgWrapperInterface_h_

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
template<typename Networks, typename Network, typename Receiver>
class NetworkCfgWrapperInterface
{
public:
    NetworkCfgWrapperInterface() {};
    virtual ~NetworkCfgWrapperInterface() {};

    std::error_code Select(Networks &networks, const char *xmlPath)
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
        shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
        if (xpathCtx == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        xmlChar *xpathExpr = (xmlChar*)"/root/network[*]";
        shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), 
                                            xmlXPathObjectDeleter()); 
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes == nullptr)
        {
            return  make_error_code(std::errc::io_error);
        }

        uint_t recieverId = 0;
        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            NetId netId = GetXmlAttrValue<NetId>(node, (const xmlChar*)"netid");

            Network *network = Network::CreateInstance(netId);
            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                SharedXmlChar ip = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"ip");

                struct sockaddr_in socketAddr;
                socketAddr.sin_family = AF_INET;
                socketAddr.sin_addr.s_addr = inet_addr((char *)ip.get());
                socketAddr.sin_port = htons(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"port"));

                xmlNodePtr tsIdNode = xmlFirstElementChild(node);
                TsId tsId = GetXmlContent<TsId>(tsIdNode);
                
                Receiver *receiver = Receiver::CreateInstance(tsId, socketAddr);
                network->Add(receiver);
            }

            networks.Add(network);
        } //for (int i = 0; i < nodes->nodeNr; ++i)

        xmlCleanupParser();
        return std::error_code();
    }
};

#pragma warning(pop)
#endif