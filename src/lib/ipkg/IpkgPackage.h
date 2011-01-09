#ifndef IPKGPACKAGE_H_
#define IPKGPACKAGE_H_

extern "C" {
#include <opkg.h>
}

class IpkgPackage
{
public:
	IpkgPackage(pkg_t *pkg);
	virtual ~IpkgPackage();

	char *name;
	char *version;
	char *architecture;
	char *section;
	char *maintainer;
	char *description;
	int installed;
	unsigned long size;
};

#endif /* IPKGPACKAGE_H_ */
