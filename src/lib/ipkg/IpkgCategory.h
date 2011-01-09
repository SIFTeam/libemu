#ifndef IPKGCATEGORY_H_
#define IPKGCATEGORY_H_

#include <list>
#include <pthread.h>
#include "IpkgPackage.h"

extern "C" {
#include <opkg.h>
}

class IpkgCategory
{
private:
	bool								m_Virtual;
	char*								m_Name;
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

	void			setName(const char *name);
	const char*		getName();
	void			packageAdd(IpkgPackage *package);
	void			packageFirst();
	bool			packageNext();
	IpkgPackage*	packageGet();
};

#endif /* IPKGCATEGORY_H_ */
