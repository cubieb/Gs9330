#ifndef _XmlDataWrapperDef_h_
#define _XmlDataWrapperDef_h_

#include "XmlDataWrapper.h"

using namespace std;

template<typename Nit>
void NitXmlWrapper<Nit>::Start() const
{
}

template<typename Nit>
error_code NitXmlWrapper<Nit>::FillNit(Nit& nit) const
{
    error_code err;

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFileName.c_str()), XmlDocDeleter());
    if (doc == nullptr)
    {
        err == system_error_t::file_not_exists;
        return err;
    }

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    nit.SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Network[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    assert(nodes != nullptr && nodes->nodeNr == 1);

    node = nodes->nodeTab[0];
    nit.SetNetworkId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ID"));
    nit.SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));
    shared_ptr<xmlChar> name = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
    nit.AddDescriptor(NetworkNameDescriptor::Tag, name.get(), strlen((const char*)name.get()) + 1);

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        AddDescriptor(nit, node, (xmlChar*)"Descriptors");

        if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId, onId;
            tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

            Nit::TransportStream& transportStream = nit.AddTransportStream(tsId, onId);
            xmlNodePtr child = xmlFirstElementChild(node);
            AddDescriptor(transportStream, child, (xmlChar*)"Descriptors");
        }
    }

    xmlCleanupParser();
    return err;
}

#endif