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

        for (int i = 0; i < nodes->nodeNr; ++i)
        {
            node = nodes->nodeTab[i];
            NetId netId = GetXmlAttrValue<NetId>(node, (const xmlChar*)"netid");
            SharedXmlChar srcIp = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"srcip");
            struct in_addr srcAddr;
            srcAddr.s_addr = inet_addr((char *)srcIp.get());

            Network *network = Network::CreateInstance(netId, srcAddr);
            for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
            {
                TsId tsId = GetXmlAttrValue<TsId>(node, (const xmlChar*)"tsid");

                SharedXmlChar dstIp = GetXmlAttrValue<SharedXmlChar>(node, (const xmlChar*)"ip");
                struct sockaddr_in dstAddr;
                dstAddr.sin_family = AF_INET;
                dstAddr.sin_addr.s_addr = inet_addr((char *)dstIp.get());
                dstAddr.sin_port = htons(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"port"));
                
                Receiver *receiver = Receiver::CreateInstance(tsId, dstAddr);                
                for (xmlNodePtr pidNode = xmlFirstElementChild(node);
                     pidNode != nullptr;
                     pidNode = xmlNextElementSibling(pidNode))
                {
                    Pid from = GetXmlAttrValue<Pid>(pidNode, (const xmlChar*)"oldpid");
                    Pid to   = GetXmlAttrValue<Pid>(pidNode, (const xmlChar*)"newpid");
                    receiver->Add(from, to);
                }

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