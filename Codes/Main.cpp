/*  history:
2015-07-06 Created by LiuHao.
*/

#include "SystemInclude.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "Common.h"
#include "Debug.h"
#include "SystemError.h" 

#include "DataWrapper.h"
#include "Main.h"

using namespace std;

struct Pserson
{
    uint_t id;
    string name;
    string email;
    string phone;
};

void PrintXpathNode(xmlNodeSetPtr nodes)
{
    size_t i, size;
    size = (nodes) ? nodes->nodeNr : 0;     

    Pserson person;
    for (i = 0; i < size; ++i)
    {        
        if(nodes->nodeTab[i]->type != XML_ELEMENT_NODE)
            continue;

        xmlNodePtr cur = nodes->nodeTab[i];   
        if (xmlStrcmp(cur->name, (xmlChar*)"Name") == 0)
            person.name = (char*)xmlNodeGetContent(cur);

        if (xmlStrcmp(cur->name, (xmlChar*)"Email") == 0)
            person.email = (char*)xmlNodeGetContent(cur);
        
    }    
    cout << person.name << ", " << person.email << endl;
}

int main(int argc, char **argv) 
{
    const char* xmlFile = "../XmlFiles/AddressListXml.xml";

    xmlInitParser();

    shared_ptr<xmlDoc> doc(xmlParseFile(xmlFile), XmlDocDeleter());
    shared_ptr<xmlXPathContext> xpathCtx(xmlXPathNewContext(doc.get()), xmlXPathContextDeleter());
    
    xmlChar *xpathExpr = (xmlChar*)"//Contact[*]";
    shared_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()), xmlXPathObjectDeleter()); 

    PrintXpathNode(xpathObj->nodesetval);
    return(0);
}
