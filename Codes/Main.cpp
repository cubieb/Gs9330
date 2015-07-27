/*  history:
2015-07-06 Created by LiuHao.
*/

#include "SystemInclude.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "Common.h"
#include "Debug.h"
#include "SystemError.h" 

#include "Main.h"

using namespace std;

class XmlDocDelter
{
public:
    XmlDocDelter()
    {}

    void operator()(xmlDocPtr doc) const
    {
        xmlFreeDoc(doc);
    }
};


//const char* XmlFileName = "../XmlFiles/C++SdkIncludeDir.xml";
const char* XmlFileName = "../XmlFiles/CreatedXml.xml";

bool CreaeXmlDoc()
{
    //定义文档和节点指针
    shared_ptr<xmlDoc> doc(xmlNewDoc((xmlChar*)"1.0"), XmlDocDelter());

    xmlNodePtr root_node = xmlNewNode(nullptr,(xmlChar*)"root");

    //设置根节点
    xmlDocSetRootElement(doc.get(),root_node);

    //在根节点中直接创建叶子节点
    xmlNewTextChild(root_node, nullptr, (xmlChar*)"newNode1", (xmlChar*)"newNode1 content");
    xmlNewTextChild(root_node, nullptr, (xmlChar*)"newNode2", (xmlChar*)"newNode2 content");
    xmlNewTextChild(root_node, nullptr, (xmlChar*)"newNode3", (xmlChar*)"newNode3 content");

    //创建一个节点，设置其内容和属性，然后加入根结点
    xmlNodePtr node = xmlNewNode(nullptr,(xmlChar*)"node2");
    xmlNodePtr content = xmlNewText((xmlChar*)"NODE CONTENT");

    xmlAddChild(root_node,node);
    xmlAddChild(node,content);
    xmlNewProp(node,(xmlChar*)"attribute",(xmlChar*)"yes");

    //创建一个儿子和孙子节点
    node = xmlNewNode(nullptr, (xmlChar*) "son");
    xmlAddChild(root_node,node);
    xmlNodePtr grandson = xmlNewNode(nullptr, (xmlChar*)"grandson");
    xmlAddChild(node,grandson);
    xmlAddChild(grandson, xmlNewText((xmlChar*)"This is a grandson node"));

    //存储xml文档
    int ret = xmlSaveFile(XmlFileName, doc.get());
    if (ret == -1)
    {
        dbgstrm << "Create XmlFile failed!" << endl;
        return false;
    }

    return true;
}

bool ReadXmlFile()
{
    shared_ptr<xmlDoc> doc(xmlReadFile(XmlFileName, "GB2312", XML_PARSE_RECOVER), XmlDocDelter());
    xmlNodePtr curNode = xmlDocGetRootElement(doc.get());   

    if (xmlStrcmp(curNode->name, (xmlChar*)"root"))
    {
        errstrm << "document of the wrong type, root node != root" << endl;
        return false;
    }

    curNode = curNode->xmlChildrenNode;
    xmlNodePtr propNodePtr = curNode;

    xmlChar *key;
    while(curNode != NULL)
    {
       //取出节点中的内容
       if ((!xmlStrcmp(curNode->name, (const xmlChar*)"newNode1")))
       {
           key = xmlNodeGetContent(curNode);
           cout << "newNode1:" << key << endl;

           xmlFree(key);
       }

       //查找带有属性attribute的节点
       if (xmlHasProp(curNode, (const xmlChar*)"attribute"))
       {
           propNodePtr = curNode;
       }

       curNode = curNode->next;
    }

    //查找属性
    xmlAttrPtr attrPtr = propNodePtr->properties;
    while (attrPtr != nullptr)
    {
       if (!xmlStrcmp(attrPtr->name, (xmlChar*)"attribute"))
       {
           xmlChar* szAttr = xmlGetProp(propNodePtr, (xmlChar*)"attribute");

           xmlFree(szAttr);
       }

       attrPtr = attrPtr->next;
    }

    return true;
}

int main()
{
    //if (!CreaeXmlDoc())
    //    return -1;

    ReadXmlFile();

	return 0;
}
