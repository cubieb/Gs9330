#ifndef _XmlDataWrapperDef_h_
#define _XmlDataWrapperDef_h_

#include "XmlDataWrapper.h"

using namespace std;

template<typename Nit>
void NitXmlWrapper<Nit>::Start() const
{
    trigger(*this);
}

template<typename Nit>
void NitXmlWrapper<Nit>::AddDescriptor(Nit& nit, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) == 0)
    {
        for (xmlNodePtr cur = xmlFirstElementChild(node); 
            cur != nullptr; 
            cur = xmlNextElementSibling(cur))
        {
            uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
            SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
            nit.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
        }
    }
}

template<typename Nit>
void NitXmlWrapper<Nit>::AddTsDescriptor(Nit& nit, uint16_t tsId,
                                         xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) == 0)
    {
        for (xmlNodePtr cur = xmlFirstElementChild(node); 
            cur != nullptr; 
            cur = xmlNextElementSibling(cur))
        {
            uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
            SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
            size_t size = strlen((const char*)data.get());
            shared_ptr<uchar_t> ascStr(new uchar_t[size/2]);
            ConvertStr2AscStr(data.get(), size, ascStr.get());

            nit.AddTsDescriptor(tsId, tag, ascStr.get(), size/2);
        }
    }
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
    nit.AddDescriptor(NetworkNameDescriptor::Tag, name.get(), strlen((const char*)name.get()));

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        AddDescriptor(nit, node, (xmlChar*)"Descriptors");

        if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");
            uint32_t freq = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Frequency");
            uint16_t fecOuter = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"Fec_Outer");
            uchar_t  modulation = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Modulation");            
            uint32_t symbolRate = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Symbol_Rate");
            uint32_t fecInner = GetXmlAttrValue<uint32_t>(node, (const xmlChar*)"Fec_Inner");

            nit.AddTs(tsId, onId);
            nit.AddTsDescriptor0x44(tsId, freq, fecOuter, modulation, symbolRate, fecInner);
            
            xmlNodePtr child = xmlFirstElementChild(node);
            AddTsDescriptor(nit, tsId, child, (xmlChar*)"Descriptors");
        }
    }

    xmlCleanupParser();
    return err;
}

#endif