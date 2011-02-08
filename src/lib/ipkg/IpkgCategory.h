#ifndef IPKGCATEGORY_H_
#define IPKGCATEGORY_H_

#include <list>
#include <pthread.h>
#include <pcre.h>
#include <libxml/tree.h>
#include "IpkgPackage.h"

extern "C" {
#include <opkg.h>
}

class IpkgCategory
{
private:
	bool								m_Virtual;
	char*								m_Name;
	char*								m_Icon;
	bool								m_Smart;
	char*								m_RuleType;
	bool								m_RuleName;
	bool								m_RuleCategory;
	char*								m_Regexp;
	pcre*								m_RegexpCompiled;
	xmlNode*							m_XmlNode;
	std::list<IpkgPackage*>				m_Packages;
	std::list<IpkgPackage*>::iterator	m_PackagesIterator;

	static bool packageSort(IpkgPackage *a, IpkgPackage *b);

public:
	IpkgCategory(const char *name);
	IpkgCategory(const char *name, bool is_virtual);
	virtual ~IpkgCategory();

	bool operator==(const char *category);

	int				count();
	int				countInstalled();
	void			sort();

	void			setName(const char *value);
	const char*		getName();
	void			setIcon(char *value);
	char*			getIcon();
	void			setRuleType(char *value);
	char*			getRuleType();
	bool			isRuleName();
	bool			isRuleCategory();
	void			setRegexp(char *value);
	char*			getRegexp();
	pcre*			getRegexpCompiled();
	void			setSmart(bool value);
	bool			isSmart();
	void			setXmlNode(xmlNode *value);
	xmlNode*		getXmlNode();
	void			packageAdd(IpkgPackage *package);
	void			packageFirst();
	bool			packageNext();
	IpkgPackage*	packageGet();
};

#endif /* IPKGCATEGORY_H_ */
