#include <iostream>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pcre.h>

extern "C" {
#include <opkg.h>
}

using namespace std;

#include "Ipkg.h"
#include "../3rd/ConfigFile.h"

pthread_mutex_t opkg_mutex = PTHREAD_MUTEX_INITIALIZER;

Ipkg::Ipkg()
{
	this->m_ProgressCallback = NULL;
	this->m_ErrorCallback = NULL;
	this->m_NoticeCallback = NULL;
	this->m_EndCallback = NULL;
	this->m_AllPackages = NULL;
	this->m_UpdatesPackages = NULL;
	this->m_XmlCategoriesCount = 0;

	this->m_XML = new IpkgXml();

	try
	{
		ConfigFile config("/etc/libsif.conf");
		config.readInto(this->m_MenuUrl, "menu_url");
		config.readInto(this->m_StbType, "stb_type");
	}
	catch (ConfigFile::file_not_found e)
	{
		// nothing to do
	}
	catch (ConfigFile::key_not_found e)
	{
		// nothing to do
	}

}

Ipkg::~Ipkg()
{
	delete this->m_XML;
}

/**********************************************************************************
 * LOG FUNCTIONS
 **********************************************************************************/

void Ipkg::sendNotice(const char *message, ...)
{
	if (this->m_NoticeCallback)
	{
		va_list args;
		char msg[16*1024];

		va_start (args, message);
		vsnprintf (msg, 16*1024, message, args);
		va_end (args);
		msg[(16*1024)-1] = '\0';

		this->m_NoticeCallback(msg, this->m_NoticeClientData);
	}
}

void Ipkg::sendError(const char *message, ...)
{
	if (this->m_ErrorCallback)
	{
		va_list args;
		char msg[16*1024];

		va_start (args, message);
		vsnprintf (msg, 16*1024, message, args);
		va_end (args);
		msg[(16*1024)-1] = '\0';

		this->m_ErrorCallback(msg, this->m_ErrorClientData);
	}
}

/**********************************************************************************
 * IPKG CALLBACKS
 **********************************************************************************/

void Ipkg::ipkgProgressCallback(const opkg_progress_data_t *progress, void *user_data)
{
	Ipkg *parent = (Ipkg*)user_data;

	if (parent->m_ProgressCallback)
		parent->m_ProgressCallback(100, progress->percentage, parent->m_ProgressClientData);
}

void Ipkg::ipkgPackageCallback(pkg_t *pkg, void *user_data)
{
	if (pkg->name)		// at least the package name !!
	{
		Ipkg *parent = (Ipkg*)user_data;

		IpkgPackage*package = new IpkgPackage(pkg);

		IpkgCategory *category = parent->categorySearch(package->section);
		if (category == NULL)
		{
			category = new IpkgCategory(package->section);
			parent->m_Categories.push_back(category);
		}

		category->packageAdd(package);
		parent->m_AllPackages->packageAdd(package);

		for (int i=0; i<parent->m_XmlCategoriesCount; i++)
		{
			if (parent->m_XmlCategories[i]->isSmart() &&
					parent->m_XmlCategories[i]->getRegexpCompiled() != NULL &&
					(parent->m_XmlCategories[i]->isRuleName() || parent->m_XmlCategories[i]->isRuleCategory()))
			{
				int ovector[1];
				int rc;
				if (parent->m_XmlCategories[i]->isRuleName())
					rc = pcre_exec(parent->m_XmlCategories[i]->getRegexpCompiled(),
							NULL, pkg->name, strlen(package->name), 0, 0, ovector, 1);
				else
					rc = pcre_exec(parent->m_XmlCategories[i]->getRegexpCompiled(),
							NULL, pkg->section, strlen(package->section), 0, 0, ovector, 1);

				if (rc == 0)
					parent->m_XmlCategories[i]->packageAdd(package);
			}
			else
			{
				xmlNode* node = NULL;
				for (node = parent->m_XmlCategories[i]->getXmlNode()->children; node; node = node->next)
				{
					if (strcmp((char*)node->name, "package") == 0)
					{
						char *tmp = parent->m_XML->getNodeAttr(node, "package");
						if (strcmp(tmp, package->name) == 0)
						{
							bool supported = false;
							char *tmp2 = parent->m_XML->getNodeAttr(node, "supported_stb");
							if (tmp2 != NULL)
							{
								char *p = strtok(tmp2, ",");
								while (p)
								{
									if (strcmp(p, parent->m_StbType.c_str()) == 0)
									{
										supported = true;
										break;
									}
									else if (strcmp(p, "all") == 0)
									{
										supported = true;
										break;
									}
									p = strtok('\0', ",");
								}

								delete tmp2;
							}

							if (supported)
							{
								tmp2 = parent->m_XML->getNodeAttr(node, "name");
								if (tmp2 != NULL)
								{
									package->setFriendlyName(tmp2);
									delete tmp2;
								}
								tmp2 = parent->m_XML->getNodeAttr(node, "icon");
								if (tmp2 != NULL)
								{
									package->setIcon(tmp2);
									delete tmp2;
								}

								xmlNode* node2 = NULL;
								int count = 0;
								for (node2 = node->children; node2; node2 = node2->next)
								{
									if (strcmp((char*)node2->name, "preview") == 0)
									{
										tmp2 = parent->m_XML->getNodeAttr(node2, "image");
										if (tmp2 != NULL)
										{
											switch (count)
											{
											case 0:
												package->setPreviewImage1(tmp2);
												break;
											case 1:
												package->setPreviewImage2(tmp2);
												break;
											case 2:
												package->setPreviewImage3(tmp2);
												break;
											case 3:
												package->setPreviewImage4(tmp2);
												break;
											case 4:
												package->setPreviewImage5(tmp2);
												break;
											}
											delete tmp2;
										}
										count++;
									}

									if (count > 4)
										break;
								}

								parent->m_XmlCategories[i]->packageAdd(package);
							}
						}
						delete tmp;
					}
				}
			}
		}
	}
}

void Ipkg::ipkgUpdatesPackageCallback(pkg_t *pkg, void *user_data)
{
	if (pkg->name)		// at least the package name !!
	{
		Ipkg *parent = (Ipkg*)user_data;
		IpkgPackage*package = new IpkgPackage(pkg);
		parent->m_UpdatesPackages->packageAdd(package);
	}
}

void Ipkg::ipkgMessageCallback(char* str, void *user_data)
{
	Ipkg *parent = (Ipkg*)user_data;
	parent->sendNotice(str);
}

void Ipkg::ipkgErrorCallback(char* str, void *user_data)
{
	Ipkg *parent = (Ipkg*)user_data;
	parent->sendError(str);
}

/**********************************************************************************
 * CALLBACKS
 **********************************************************************************/

void Ipkg::setProgressCallback(PRGCBFUNC func, void *clientdata)
{
	this->m_ProgressCallback = func;
	this->m_ProgressClientData = clientdata;
}

void Ipkg::setNoticeCallback(WRNCBFUNC func, void *clientdata)
{
	this->m_NoticeCallback = func;
	this->m_NoticeClientData = clientdata;
}

void Ipkg::setErrorCallback(ERRCBFUNC func, void *clientdata)
{
	this->m_ErrorCallback = func;
	this->m_ErrorClientData = clientdata;
}

void Ipkg::setEndCallback(ENDCBFUNC func, void *clientdata)
{
	this->m_EndCallback = func;
	this->m_EndClientData = clientdata;
}

void Ipkg::clearCallbacks()
{
	this->m_ProgressCallback = NULL;
	this->m_ErrorCallback = NULL;
	this->m_EndCallback = NULL;
	this->m_NoticeCallback = NULL;
}

/**********************************************************************************
 * MULTITHREADING
 **********************************************************************************/

void Ipkg::join()
{
	pthread_join(this->m_AsyncThread, NULL);
}

/**********************************************************************************
 * ACTIONS
 **********************************************************************************/

static const char *force_defaults_key = "force_defaults";
static const char *force_reinstall_key = "force_reinstall";
static const char *verbosity_key = "verbosity";

void *Ipkg::TUpdate(void *ptr)
{
	bool force_defaults = true;
	int verbosity = 1;
	Ipkg *parent = (Ipkg*)ptr;
	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		opkg_set_option((char*)force_defaults_key, &force_defaults);
		opkg_set_option((char*)verbosity_key, &verbosity);
		opkg_message_set_callbacks(Ipkg::ipkgMessageCallback, Ipkg::ipkgErrorCallback, ptr);
		opkg_update_package_lists(Ipkg::ipkgProgressCallback, ptr);

		opkg_message_unset_callbacks();
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);

	/* xml for customized categories */
	if (!parent->m_XML->readXML(parent->m_MenuUrl.c_str()))
		parent->sendError("Cannot get SIFTeam preferred categories");
	parent->m_XmlCategoriesCount = 0;

	if (parent->m_EndCallback)
		parent->m_EndCallback(true, parent->m_EndClientData);

	return NULL;
}

void *Ipkg::TDownload(void *ptr)
{
	bool force_defaults = true;
	int verbosity = 1;
	Ipkg *parent = (Ipkg*)ptr;

	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		opkg_set_option((char*)force_defaults_key, &force_defaults);
		opkg_set_option((char*)verbosity_key, &verbosity);
		opkg_message_set_callbacks(Ipkg::ipkgMessageCallback, Ipkg::ipkgErrorCallback, ptr);
		opkg_download_package(parent->m_AsyncArgs, Ipkg::ipkgProgressCallback, ptr);

		opkg_message_unset_callbacks();
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);

	if (parent->m_EndCallback)
		parent->m_EndCallback(true, parent->m_EndClientData);

	return NULL;
}

void *Ipkg::TInstall(void *ptr)
{
	bool force_defaults = true;
	int verbosity = 1;
	Ipkg *parent = (Ipkg*)ptr;

	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		opkg_set_option((char*)force_defaults_key, &force_defaults);
		opkg_set_option((char*)force_reinstall_key, &force_defaults);
		opkg_set_option((char*)verbosity_key, &verbosity);
		opkg_message_set_callbacks(Ipkg::ipkgMessageCallback, Ipkg::ipkgErrorCallback, ptr);
		opkg_install_package(parent->m_AsyncArgs, Ipkg::ipkgProgressCallback, ptr);

		opkg_message_unset_callbacks();
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);

	if (parent->m_EndCallback)
		parent->m_EndCallback(true, parent->m_EndClientData);

	return NULL;
}

void *Ipkg::TRemove(void *ptr)
{
	bool force_defaults = true;
	int verbosity = 1;
	Ipkg *parent = (Ipkg*)ptr;

	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		opkg_set_option((char*)force_defaults_key, &force_defaults);
		opkg_set_option((char*)verbosity_key, &verbosity);
		opkg_message_set_callbacks(Ipkg::ipkgMessageCallback, Ipkg::ipkgErrorCallback, ptr);
		opkg_remove_package(parent->m_AsyncArgs, Ipkg::ipkgProgressCallback, ptr);

		opkg_message_unset_callbacks();
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);

	if (parent->m_EndCallback)
		parent->m_EndCallback(true, parent->m_EndClientData);

	return NULL;
}

void *Ipkg::TUpgrade(void *ptr)
{
	bool force_defaults = true;
	int verbosity = 1;
	Ipkg *parent = (Ipkg*)ptr;

	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		opkg_set_option((char*)force_defaults_key, &force_defaults);
		opkg_set_option((char*)force_reinstall_key, &force_defaults);
		opkg_set_option((char*)verbosity_key, &verbosity);
		opkg_message_set_callbacks(Ipkg::ipkgMessageCallback, Ipkg::ipkgErrorCallback, ptr);
		opkg_upgrade_all(Ipkg::ipkgProgressCallback, ptr);

		opkg_message_unset_callbacks();
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);

	if (parent->m_EndCallback)
		parent->m_EndCallback(true, parent->m_EndClientData);

	return NULL;
}

void Ipkg::update()
{
	pthread_create(&this->m_AsyncThread, NULL, Ipkg::TUpdate, (void*)this);
}

void Ipkg::download(char *package)
{
	memset(this->m_AsyncArgs, 0, 256);
	strncpy(this->m_AsyncArgs, package, 255);
	pthread_create(&this->m_AsyncThread, NULL, Ipkg::TDownload, (void*)this);
}

void Ipkg::install(char *package)
{
	memset(this->m_AsyncArgs, 0, 256);
	strncpy(this->m_AsyncArgs, package, 255);
	pthread_create(&this->m_AsyncThread, NULL, Ipkg::TInstall, (void*)this);
}

void Ipkg::remove(char *package)
{
	memset(this->m_AsyncArgs, 0, 256);
	strncpy(this->m_AsyncArgs, package, 255);
	pthread_create(&this->m_AsyncThread, NULL, Ipkg::TRemove, (void*)this);
}

void Ipkg::upgrade()
{
	pthread_create(&this->m_AsyncThread, NULL, Ipkg::TUpgrade, (void*)this);
}

bool Ipkg::isUpgradeable()
{
	bool ret = false;
	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		ret = (opkg_is_upgradable() == 1);
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);
	return ret;
}

/**********************************************************************************
 * CATEGORIES HANDLING
 **********************************************************************************/

void Ipkg::categoryInit()
{
	this->m_AllPackages = new IpkgCategory("all packages", true);
	this->m_UpdatesPackages = new IpkgCategory("updates");		// this is real.. not virtual

	/* xml for customized categories */
	this->m_XmlCategoriesCount = this->m_XML->getCategoriesCount();
	this->m_XmlCategories = new IpkgCategory* [this->m_XmlCategoriesCount];
	for (int i=0; i<this->m_XmlCategoriesCount; i++)
	{
		xmlNode* node = this->m_XML->getCategoryNode(i);
		char *tmp = this->m_XML->getNodeAttr(node, "name");
		if (tmp != NULL)
		{
			this->m_XmlCategories[i] = new IpkgCategory(tmp, true);
			delete tmp;
		}
		else
			this->m_XmlCategories[i] = new IpkgCategory("unknow", true);

		this->m_XmlCategories[i]->setXmlNode(node);
		tmp = this->m_XML->getNodeAttr(node, "icon");
		if (tmp != NULL)
		{
			this->m_XmlCategories[i]->setIcon(tmp);
			delete tmp;
		}

		tmp = this->m_XML->getNodeAttr(node, "type");
		if (tmp != NULL)
		{
			if (strcmp(tmp, "smart") == 0)
			{
				this->m_XmlCategories[i]->setSmart(true);

				char *tmp2 = this->m_XML->getNodeAttr(node, "ruletype");
				if (tmp2 != NULL)
				{
					this->m_XmlCategories[i]->setRuleType(tmp2);
					delete tmp2;
				}

				tmp2 = this->m_XML->getNodeAttr(node, "regexp");
				if (tmp2 != NULL)
				{
					this->m_XmlCategories[i]->setRegexp(tmp2);
					delete tmp2;
				}
			}
			delete tmp;
		}
	}

	pthread_mutex_lock(&opkg_mutex);
	if (opkg_new() == 0)
	{
		opkg_list_packages(Ipkg::ipkgPackageCallback, (void*)this);
		opkg_list_upgradable_packages(Ipkg::ipkgUpdatesPackageCallback, (void*)this);
		opkg_free();
	}
	pthread_mutex_unlock(&opkg_mutex);

	std::list<IpkgCategory*>::iterator it;
	for (it=this->m_Categories.begin(); it != this->m_Categories.end(); it++)
		(*it)->sort();

	this->m_AllPackages->sort();
	//this->m_PluginsPackages->sort();
	//this->m_SettingsPackages->sort();
	//this->m_EmulatorsPackages->sort();
	//this->m_ExtraPackages->sort();
	this->m_UpdatesPackages->sort();

	this->m_Categories.sort(Ipkg::categorySort);
	this->m_CategoriesIterator = this->m_Categories.begin();
}

void Ipkg::categoryDeinit()
{
	std::list<IpkgCategory*>::iterator it;
	for (it=this->m_Categories.begin(); it != this->m_Categories.end(); it++)
		delete *it;

	delete this->m_AllPackages;
	delete this->m_UpdatesPackages;

	for (int i=0; i<this->m_XmlCategoriesCount; i++)
		delete this->m_XmlCategories[i];

	delete this->m_XmlCategories;

	this->m_Categories.clear();
}

void Ipkg::categoryRefresh()
{
	this->categoryDeinit();
	this->categoryInit();
}

IpkgCategory* Ipkg::categoryGetAll()
{
	return this->m_AllPackages;
}

IpkgCategory* Ipkg::categoryGetUpdates()
{
	return this->m_UpdatesPackages;
}

bool Ipkg::categorySort(IpkgCategory *a, IpkgCategory *b)
{
	unsigned int i = 0;
	const char *tmpA = a->getName();
	const char *tmpB = b->getName();
	while (i<strlen(tmpA) && i<strlen(tmpB))
	{
		if (tolower(tmpA[i]) < tolower(tmpB[i])) return true;
		else if (tolower(tmpA[i]) > tolower(tmpB[i])) return false;
		++i;
	}
	if (strlen(tmpA) < strlen(tmpB)) return true;
	else return false;
}

void Ipkg::categoryFirst()
{
	this->m_CategoriesIterator = this->m_Categories.begin();
}

bool Ipkg::categoryNext()
{
	this->m_CategoriesIterator++;
	if (this->m_CategoriesIterator == this->m_Categories.end())
		return false;

	return true;
}

IpkgCategory* Ipkg::categoryGet()
{
	return *this->m_CategoriesIterator;
}

IpkgCategory* Ipkg::categorySearch(const char *name)
{
	std::list<IpkgCategory*>::iterator it;

	for (it=this->m_Categories.begin(); it != this->m_Categories.end(); it++)
		if (**it == name)
			return *it;

	return NULL;
}

int Ipkg::categoryCount()
{
	return this->m_Categories.size();
}

int Ipkg::xmlCategoriesCount()
{
	return this->m_XmlCategoriesCount;
}

IpkgCategory* Ipkg::categoryGetXml(int id)
{
	return this->m_XmlCategories[id];
}
