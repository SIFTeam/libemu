#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "IpkgCategory.h"

IpkgCategory::IpkgCategory(const char *name)
{
	this->m_Name = new char[strlen(name)+ 1];
	strcpy(this->m_Name, name);
	this->m_Virtual = false;
	this->m_Smart = false;
	this->m_Icon = new char[1];
	this->m_Icon[0] = '\0';
	this->m_RuleType = new char[1];
	this->m_RuleType[0] = '\0';
	this->m_RuleName = false;
	this->m_RuleCategory = false;
	this->m_Regexp = new char[1];
	this->m_Regexp[0] = '\0';
	this->m_RegexpCompiled = NULL;
	this->m_XmlNode = NULL;
}

IpkgCategory::IpkgCategory(const char *name, bool is_virtual)
{
	this->m_Name = new char[strlen(name)+ 1];
	strcpy(this->m_Name, name);
	this->m_Virtual = is_virtual;
	this->m_Smart= false;
	this->m_Icon = new char[1];
	this->m_Icon[0] = '\0';
	this->m_RuleType = new char[1];
	this->m_RuleType[0] = '\0';
	this->m_RuleName = false;
	this->m_RuleCategory = false;
	this->m_Regexp = new char[1];
	this->m_Regexp[0] = '\0';
	this->m_RegexpCompiled = NULL;
	this->m_XmlNode = NULL;
}

IpkgCategory::~IpkgCategory()
{
	if (!this->m_Virtual)
	{
		std::list<IpkgPackage*>::iterator it;
		for (it = this->m_Packages.begin(); it != this->m_Packages.end(); it++)
			delete *it;
	}

	if (this->m_Name) delete this->m_Name;
	if (this->m_Icon) delete this->m_Icon;
	if (this->m_RuleType) delete this->m_RuleType;
	if (this->m_Regexp) delete this->m_Regexp;

	if (this->m_RegexpCompiled)
		pcre_free(this->m_RegexpCompiled);
}

bool IpkgCategory::operator==(const char *category)
{
	if (strlen(this->m_Name) == strlen(category))
		if (memcmp(this->m_Name, category, strlen(category)) == 0)
			return true;
	return false;
}


int IpkgCategory::count()
{
	int ret = 0;
	std::list<IpkgPackage*>::iterator it;
	for (it = this->m_Packages.begin(); it != this->m_Packages.end(); it++)
		ret++;

	return ret;
}

int IpkgCategory::countInstalled()
{
	int ret = 0;

	std::list<IpkgPackage*>::iterator it;
	for (it = this->m_Packages.begin(); it != this->m_Packages.end(); it++)
	{
		IpkgPackage* tmp = *it;
		if (tmp->installed) ret++;
	}

	return ret;
}

void IpkgCategory::setName(const char *value)
{
	if (this->m_Name) delete this->m_Name;
	this->m_Name = new char[strlen(value)+ 1];
	strcpy(this->m_Name, value);
}

const char *IpkgCategory::getName()
{
	return this->m_Name;
}

void IpkgCategory::setIcon(char *value)
{
	if (this->m_Icon) delete this->m_Icon;
	this->m_Icon = new char[strlen(value)+ 1];
	strcpy(this->m_Icon, value);
}

char *IpkgCategory::getIcon()
{
	return this->m_Icon;
}

void IpkgCategory::setRuleType(char *value)
{
	if (this->m_RuleType) delete this->m_RuleType;
	this->m_RuleType = new char[strlen(value)+ 1];
	strcpy(this->m_RuleType, value);

	this->m_RuleName = false;
	this->m_RuleCategory = false;
	if (strcmp(this->m_RuleType, "name") == 0)
		this->m_RuleName = true;
	else if (strcmp(this->m_RuleType, "category") == 0)
		this->m_RuleCategory = true;
}

char *IpkgCategory::getRuleType()
{
	return this->m_RuleType;
}

bool IpkgCategory::isRuleName()
{
	return this->m_RuleName;
}

bool IpkgCategory::isRuleCategory()
{
	return this->m_RuleCategory;
}

void IpkgCategory::setRegexp(char *value)
{
	if (this->m_Regexp) delete this->m_Regexp;
	this->m_Regexp = new char[strlen(value)+ 1];
	strcpy(this->m_Regexp, value);

	const char *error;
	int erroffset;
	if (this->m_RegexpCompiled)
		pcre_free(this->m_RegexpCompiled);
	this->m_RegexpCompiled = pcre_compile(this->m_Regexp, 0, &error, &erroffset, NULL);
}

char *IpkgCategory::getRegexp()
{
	return this->m_Regexp;
}

pcre *IpkgCategory::getRegexpCompiled()
{
	return this->m_RegexpCompiled;
}

void IpkgCategory::setSmart(bool value)
{
	this->m_Smart = value;
}

bool IpkgCategory::isSmart()
{
	return this->m_Smart;
}

void IpkgCategory::setXmlNode(xmlNode *value)
{
	this->m_XmlNode = value;
}

xmlNode* IpkgCategory::getXmlNode()
{
	return this->m_XmlNode;
}

bool IpkgCategory::packageSort(IpkgPackage *a, IpkgPackage *b)
{
	unsigned int i = 0;
	const char *tmpA = a->name;
	const char *tmpB = b->name;
	while (i<strlen(tmpA) && i<strlen(tmpB))
	{
		if (tolower(tmpA[i]) < tolower(tmpB[i])) return true;
		else if (tolower(tmpA[i]) > tolower(tmpB[i])) return false;
		++i;
	}
	if (strlen(tmpA) < strlen(tmpB)) return true;
	else return false;
}

void IpkgCategory::sort()
{
	this->m_Packages.sort(IpkgCategory::packageSort);
	this->m_PackagesIterator = this->m_Packages.begin();
}

void IpkgCategory::packageAdd(IpkgPackage *package)
{
	this->m_Packages.push_back(package);
}

void IpkgCategory::packageFirst()
{
	this->m_PackagesIterator = this->m_Packages.begin();
}

bool IpkgCategory::packageNext()
{
	this->m_PackagesIterator++;
	if (this->m_PackagesIterator == this->m_Packages.end())
		return false;
	return true;
}

IpkgPackage* IpkgCategory::packageGet()
{
	return *this->m_PackagesIterator;
}
