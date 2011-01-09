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
}
