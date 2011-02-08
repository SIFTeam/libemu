#ifndef IPKGXML_H_
#define IPKGXML_H_

#include <list>
#include <libxml/parser.h>
#include <libxml/tree.h>

class IpkgXml
{
private:
	xmlDocPtr	m_Doc;

public:
	IpkgXml();
	virtual ~IpkgXml();

	bool		readXML(const char *url);
	void		closeXML();
	void		dumpNode();
	void		dumpNode(xmlNode * a_node);

	int			getCategoriesCount();
	xmlNode*	getCategoryNode(int id);

	char*		getNodeAttr(xmlNode *node, const char *attr);
};

#endif /* IPKGXML_H_ */
