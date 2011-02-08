#include <iostream>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

using namespace std;

#include "IpkgXml.h"

IpkgXml::IpkgXml()
{
	LIBXML_TEST_VERSION;
	this->m_Doc = NULL;
}

IpkgXml::~IpkgXml()
{
	if (this->m_Doc != NULL)
		xmlFreeDoc(this->m_Doc);

	xmlCleanupParser();
}

bool IpkgXml::readXML(const char *url)
{
	if (this->m_Doc != NULL)
		xmlFreeDoc(this->m_Doc);

	//xmlReadMemory(content, length, "noname.xml", NULL, 0);
	this->m_Doc = xmlReadFile((char*)url, NULL, 0);
	if (this->m_Doc == NULL)
		return false;

	return true;
}

void IpkgXml::closeXML()
{
	if (this->m_Doc != NULL)
		xmlFreeDoc(this->m_Doc);

	this->m_Doc = NULL;
}

void IpkgXml::dumpNode(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
			printf("node type: Element, name: %s\n", cur_node->name);

		this->dumpNode(cur_node->children);
	}
}

void IpkgXml::dumpNode()
{
	this->dumpNode(xmlDocGetRootElement(this->m_Doc));
}

int IpkgXml::getCategoriesCount()
{
	if (this->m_Doc == NULL)
		return 0;

	int count = 0;
	xmlNode *node = xmlDocGetRootElement(this->m_Doc);
	xmlNode *pnode = NULL;

	for (pnode = node; pnode; pnode = pnode->next)
	{
		if (pnode->type == XML_ELEMENT_NODE)
			if (strcmp((const char*)pnode->name, "categories") == 0)
			{
				xmlNode *pnode2 = NULL;
				for (pnode2 = pnode->children; pnode2; pnode2 = pnode2->next)
				{
					if (pnode2->type == XML_ELEMENT_NODE)
						if (strcmp((const char*)pnode2->name, "category") == 0)
							count ++;
				}
				break;
			}
	}

	return count;
}

char* IpkgXml::getNodeAttr(xmlNode *node, const char *attr)
{
	if (node == NULL)
		return NULL;

	return (char*)xmlGetProp(node, (xmlChar*)attr);
}

xmlNode* IpkgXml::getCategoryNode(int id)
{
	if (this->m_Doc == NULL)
		return NULL;

	int count = 0;
	xmlNode *node = xmlDocGetRootElement(this->m_Doc);
	xmlNode *pnode = NULL;

	for (pnode = node; pnode; pnode = pnode->next)
	{
		if (pnode->type == XML_ELEMENT_NODE)
			if (strcmp((const char*)pnode->name, "categories") == 0)
			{
				xmlNode *pnode2 = NULL;
				for (pnode2 = pnode->children; pnode2; pnode2 = pnode2->next)
				{
					if (pnode2->type == XML_ELEMENT_NODE)
						if (strcmp((const char*)pnode2->name, "category") == 0)
						{
							if (count == id)
								return pnode2;
							count ++;
						}
				}
				break;
			}
	}

	return NULL;
}
