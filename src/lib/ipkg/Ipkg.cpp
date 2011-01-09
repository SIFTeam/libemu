#include <iostream>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

extern "C" {
#include <opkg.h>
}

using namespace std;

#include "Ipkg.h"

pthread_mutex_t opkg_mutex = PTHREAD_MUTEX_INITIALIZER;

Ipkg::Ipkg()
{
	this->m_ProgressCallback = NULL;
	this->m_ErrorCallback = NULL;
	this->m_NoticeCallback = NULL;
	this->m_EndCallback = NULL;
	this->m_AllPackages = NULL;
	this->m_PluginsPackages = NULL;
	this->m_SettingsPackages = NULL;
	this->m_EmulatorsPackages = NULL;
	this->m_ExtraPackages = NULL;
	this->m_UpdatesPackages = NULL;
}

Ipkg::~Ipkg()
{

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

		if (strlen(package->name) > 15)
			if (memcmp(package->name, "enigma2-plugin-", 15) == 0)
				parent->m_PluginsPackages->packageAdd(package);

		if (strcmp(package->section, "settings") == 0)
			parent->m_SettingsPackages->packageAdd(package);

		if (strcmp(package->section, "emu") == 0 || strcmp(package->section, "emulators") == 0 || strcmp(package->section, "emucam") == 0)
			parent->m_EmulatorsPackages->packageAdd(package);

		if (strcmp(package->section, "extra") == 0)
			parent->m_ExtraPackages->packageAdd(package);
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

void Ipkg::ipkgMessageCallback(const char* str, void *user_data)
{
	Ipkg *parent = (Ipkg*)user_data;
	parent->sendNotice(str);
}

void Ipkg::ipkgErrorCallback(const char* str, void *user_data)
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
	this->m_PluginsPackages = new IpkgCategory("plugins", true);
	this->m_SettingsPackages = new IpkgCategory("settings", true);
	this->m_EmulatorsPackages = new IpkgCategory("emulators", true);
	this->m_ExtraPackages = new IpkgCategory("extra", true);
	this->m_UpdatesPackages = new IpkgCategory("updates");		// this is real.. not virtual

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
	this->m_PluginsPackages->sort();
	this->m_SettingsPackages->sort();
	this->m_EmulatorsPackages->sort();
	this->m_ExtraPackages->sort();
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
	delete this->m_PluginsPackages;
	delete this->m_SettingsPackages;
	delete this->m_EmulatorsPackages;
	delete this->m_ExtraPackages;
	delete this->m_UpdatesPackages;

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

IpkgCategory* Ipkg::categoryGetPlugins()
{
	return this->m_PluginsPackages;
}

IpkgCategory* Ipkg::categoryGetSettings()
{
	return this->m_SettingsPackages;
}

IpkgCategory* Ipkg::categoryGetEmulators()
{
	return this->m_EmulatorsPackages;
}

IpkgCategory* Ipkg::categoryGetExtra()
{
	return this->m_ExtraPackages;
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
