#ifndef _XmlDataWrapperDef_h_
#define _XmlDataWrapperDef_h_

#include "XmlDataWrapper.h"

using namespace std;

/**********************class NitXmlWrapper**********************/
template<typename Table>
void NitXmlWrapper<Table>::AddDescriptor(Table& nit, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        nit.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
    }   
}

template<typename Table>
void NitXmlWrapper<Table>::AddTsDescriptor(Table& nit, uint16_t tsId,
                                         xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

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

template<typename Table>
error_code NitXmlWrapper<Table>::Fill(Table& nit) const
{
    error_code err;

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFileName.c_str()), XmlDocDeleter());
    if (doc == nullptr)
    { 
        err = system_error_t::file_not_exists;
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

/**********************class SdtXmlWrapper**********************/
template<typename Table>
void SdtXmlWrapper<Table>::AddService(Table& sdt, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    uint16_t serviceId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ServiceID");
    uchar_t  eitScheduleFlag = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"EIT_schedule_flag");
    uchar_t  eitPresentFollowingFlag = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"EIT_present_following_flag");
    uchar_t  runningStatus = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"running_status");
    uchar_t  freeCaMode = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"free_CA_mode");
    sdt.AddService(serviceId, eitScheduleFlag, eitPresentFollowingFlag, runningStatus, freeCaMode);
    
    shared_ptr<xmlChar> serviceName = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
    shared_ptr<xmlChar> providerName = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Provider_Name");
    uchar_t type = GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Type");
    sdt.AddServiceDescriptor0x48(serviceId, type, providerName.get(), serviceName.get());

    for (xmlNodePtr cur = xmlFirstElementChild(xmlFirstElementChild(node)); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        sdt.AddServiceDescriptor(serviceId, tag, data.get(), strlen((const char*)data.get()));
    }
}

template<typename Table>
error_code SdtXmlWrapper<Table>::Fill(Table& sdt) const
{
    error_code err;

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFileName.c_str()), XmlDocDeleter());
    if (doc == nullptr)
    {
        err = system_error_t::file_not_exists;
        return err;
    }

    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    sdt.SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Transportstream[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    
    for (int i = 0; i < nodes->nodeNr; ++i)
    {
        node = nodes->nodeTab[i];
        sdt.SetTsId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID"));
        sdt.SetOnId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID"));
        sdt.SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));

        for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
        {
            AddService(sdt, node, (xmlChar*)"Service");      
        }
    }

    xmlCleanupParser();
    return err;
}

/**********************class BatXmlWrapper**********************/
template<typename Table>
void BatXmlWrapper<Table>::AddDescriptor(Table& bat, xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        bat.AddDescriptor(tag, data.get(), strlen((const char*)data.get()));
    }   
}

template<typename Table>
void BatXmlWrapper<Table>::AddTsDescriptor(Table& bat, uint16_t tsId,
                                         xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uchar_t tag = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Tag");
        SharedXmlChar data = GetXmlAttrValue<SharedXmlChar>(cur, (const xmlChar*)"Data");
        size_t size = strlen((const char*)data.get());
        shared_ptr<uchar_t> ascStr(new uchar_t[size/2]);
        ConvertStr2AscStr(data.get(), size, ascStr.get());

        bat.AddTsDescriptor(tsId, tag, ascStr.get(), size/2);
    }
}

template<typename Table>
void BatXmlWrapper<Table>::AddTsDescriptor0x41(Table& bat, uint16_t tsId,
                                         xmlNodePtr& node, xmlChar* child) const
{
    if (xmlStrcmp(node->name, child) != 0)
    {
        return;
    }

    list<pair<uint16_t, uchar_t>> serviceList;
    for (xmlNodePtr cur = xmlFirstElementChild(node); 
        cur != nullptr; 
        cur = xmlNextElementSibling(cur))
    {
        uint16_t service = GetXmlAttrValue<uint16_t>(cur, (const xmlChar*)"ID");
        uchar_t type = GetXmlAttrValue<uchar_t>(cur, (const xmlChar*)"Type");
        serviceList.push_back(make_pair(service, type));
    }
    bat.AddTsDescriptor0x41(tsId, serviceList);
}

template<typename Table>
error_code BatXmlWrapper<Table>::Fill(Table& bat) const
{
    error_code err;

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFileName.c_str()), XmlDocDeleter());
    if (doc == nullptr)
    { 
        err = system_error_t::file_not_exists;
        return err;
    }
     
    xmlNodePtr node = xmlDocGetRootElement(doc.get());
    bat.SetTableId(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"TableID"));

    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    assert(xpathCtx != nullptr);

    xmlChar *xpathExpr = (xmlChar*)"/Root/Bouquet[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    assert(nodes != nullptr && nodes->nodeNr == 1);

    node = nodes->nodeTab[0];
    bat.SetBouquetId(GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"BouquetID"));
    bat.SetVersionNumber(GetXmlAttrValue<uchar_t>(node, (const xmlChar*)"Version"));
    shared_ptr<xmlChar> name = GetXmlAttrValue<shared_ptr<xmlChar>>(node, (const xmlChar*)"Name");
    bat.AddDescriptor(BouquetNameDescriptor::Tag, name.get(), strlen((const char*)name.get()));

    for (node = xmlFirstElementChild(node); node != nullptr; node = xmlNextElementSibling(node))
    {
        AddDescriptor(bat, node, (xmlChar*)"Descriptors");

        if (xmlStrcmp(node->name, (xmlChar*)"Transportstream") == 0)
        {
            uint16_t tsId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"TSID");
            uint16_t onId = GetXmlAttrValue<uint16_t>(node, (const xmlChar*)"ONID");

            bat.AddTs(tsId, onId);
            
            xmlNodePtr child = xmlFirstElementChild(node);
            AddTsDescriptor0x41(bat, tsId, child, (xmlChar*)"ServiceList");

            child = xmlNextElementSibling(child);
            AddTsDescriptor(bat, tsId, child, (xmlChar*)"Descriptors");
        }
    }
    xmlCleanupParser();
    return err;
}

#endif