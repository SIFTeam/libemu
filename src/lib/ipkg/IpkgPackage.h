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

	void setIcon(char* value);
	void setFriendlyName(char* value);
	void setPreviewImage1(char* value);
	void setPreviewImage2(char* value);
	void setPreviewImage3(char* value);
	void setPreviewImage4(char* value);
	void setPreviewImage5(char* value);

	char *name;
	char *version;
	char *architecture;
	char *section;
	char *maintainer;
	char *description;
	int installed;
	unsigned long size;
	char *icon;
	char *friendlyname;
	char *previewimage1;
	char *previewimage2;
	char *previewimage3;
	char *previewimage4;
	char *previewimage5;
};

#endif /* IPKGPACKAGE_H_ */
