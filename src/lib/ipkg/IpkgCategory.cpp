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
}

IpkgCategory::IpkgCategory(const char *name, bool is_virtual)
{
	this->m_Name = new char[strlen(name)+ 1];
	strcpy(this->m_Name, name);
	this->m_Virtual = is_virtual;
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

void IpkgCategory::setName(const char *name)
{
	if (this->m_Name) delete this->m_Name;
	this->m_Name = new char[strlen(name)+ 1];
	strcpy(this->m_Name, name);
}

const char *IpkgCategory::getName()
{
	return this->m_Name;
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
