#ifndef _TimerCfgWrapperInterface_h_
#define _TimerCfgWrapperInterface_h_

#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/Deleter.h"
#include "Include/Foundation/XmlHelper.h"

template<typename TimerCfg>
class TimerCfgWrapperInterface
{
public:
    TimerCfgWrapperInterface() {};
    virtual ~TimerCfgWrapperInterface() {};

    std::error_code Select(TimerCfg &timerCfg, const char *xmlPath)
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

        for (node = xmlFirstElementChild(xmlFirstElementChild(node)); 
             node != nullptr; 
             node = xmlNextElementSibling(node))
        {
            if (xmlStrcmp(node->name, (xmlChar*)"nitactual") == 0)
            {
                timerCfg.SetInterval(NitActualTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"nitothers") == 0)
            {
                timerCfg.SetInterval(NitOtherTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"sdtactual") == 0)
            {
                timerCfg.SetInterval(SdtActualTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"sdtothers") == 0)
            {
                timerCfg.SetInterval(SdtOtherTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"bat") == 0)
            {
                timerCfg.SetInterval(BatTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"eitactualpf") == 0)
            {
                timerCfg.SetInterval(EitActualPfTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"eitotherspf") == 0)
            {
                timerCfg.SetInterval(EitOtherPfTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"eitactualsch") == 0)
            {
                timerCfg.SetInterval(EitActualSchTableId, GetXmlContent<time_t>(node));
            }
            else if (xmlStrcmp(node->name, (xmlChar*)"eitothersch") == 0)
            {
                timerCfg.SetInterval(EitOtherSchTableId, GetXmlContent<time_t>(node));
            }
        }

        return std::error_code();
    }
};

#endif