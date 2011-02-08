#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "IpkgPackage.h"

IpkgPackage::IpkgPackage(pkg_t *pkg)
{
	name = new char[strlen(pkg->name)+1];
	strcpy(name, pkg->name);

	if (pkg->section)
	{
		section = new char[strlen(pkg->section)+1];
		strcpy(section, pkg->section);
	}
	else
	{
		section = new char[6];
		sprintf(section, "other");
	}

	if (pkg->version)
	{
		version = new char[strlen(pkg->version)+1];
		strcpy(version, pkg->version);
	}
	else
	{
		version = new char[1];
		version[0] = '\0';
	}

	if (pkg->architecture)
	{
		architecture = new char[strlen(pkg->architecture)+1];
		strcpy(architecture, pkg->architecture);
	}
	else
	{
		architecture = new char[1];
		architecture[0] = '\0';
	}

	if (pkg->description)
	{
		description = new char[strlen(pkg->description)+1];
		strcpy(description, pkg->description);
	}
	else
	{
		description = new char[1];
		description[0] = '\0';
	}

	icon = new char[1];
	icon[0] = '\0';

	friendlyname = new char[1];
	friendlyname[0] = '\0';

	previewimage1 = new char[1];
	previewimage1[0] = '\0';

	previewimage2 = new char[1];
	previewimage2[0] = '\0';

	previewimage3 = new char[1];
	previewimage3[0] = '\0';

	previewimage4 = new char[1];
	previewimage4[0] = '\0';

	previewimage5 = new char[1];
	previewimage5[0] = '\0';

	size = pkg->size;
	installed = ((pkg->state_status & SS_INSTALLED) != 0);
}

IpkgPackage::~IpkgPackage()
{
	delete name;
	delete section;
	delete version;
	delete architecture;
	delete description;
	delete icon;
	delete friendlyname;
	delete previewimage1;
	delete previewimage2;
	delete previewimage3;
	delete previewimage4;
	delete previewimage5;
}

void IpkgPackage::setIcon(char* value)
{
	delete icon;
	icon = new char[strlen(value)+1];
	strcpy(icon, value);
}

void IpkgPackage::setFriendlyName(char* value)
{
	delete friendlyname;
	friendlyname = new char[strlen(value)+1];
	strcpy(friendlyname, value);
}

void IpkgPackage::setPreviewImage1(char* value)
{
	delete previewimage1;
	previewimage1 = new char[strlen(value)+1];
	strcpy(previewimage1, value);
}

void IpkgPackage::setPreviewImage2(char* value)
{
	delete previewimage2;
	previewimage2 = new char[strlen(value)+1];
	strcpy(previewimage2, value);
}

void IpkgPackage::setPreviewImage3(char* value)
{
	delete previewimage3;
	previewimage3 = new char[strlen(value)+1];
	strcpy(previewimage3, value);
}

void IpkgPackage::setPreviewImage4(char* value)
{
	delete previewimage4;
	previewimage4 = new char[strlen(value)+1];
	strcpy(previewimage4, value);
}

void IpkgPackage::setPreviewImage5(char* value)
{
	delete previewimage5;
	previewimage5 = new char[strlen(value)+1];
	strcpy(previewimage5, value);
}
