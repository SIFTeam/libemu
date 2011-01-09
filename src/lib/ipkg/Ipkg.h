#ifndef IPKG_H_
#define IPKG_H_

#include <list>
#include <pthread.h>

#include "IpkgCategory.h"

typedef double (*PRGCBFUNC)(double, double, void *);
typedef void (*WRNCBFUNC)(char *, void *);
typedef void (*ERRCBFUNC)(char *, void *);
typedef void (*ENDCBFUNC)(bool, void *);

class Ipkg
{
private:
	PRGCBFUNC	m_ProgressCallback;
	void*		m_ProgressClientData;
	WRNCBFUNC	m_NoticeCallback;
	void*		m_NoticeClientData;
	ERRCBFUNC	m_ErrorCallback;
	void*		m_ErrorClientData;
	ENDCBFUNC	m_EndCallback;
	void*		m_EndClientData;

	pthread_t	m_AsyncThread;
	char		m_AsyncArgs[256];

	std::list<IpkgCategory*>			m_Categories;
	std::list<IpkgCategory*>::iterator	m_CategoriesIterator;

	IpkgCategory*	m_AllPackages;
	IpkgCategory*	m_PluginsPackages;
	IpkgCategory*	m_SettingsPackages;
	IpkgCategory*	m_EmulatorsPackages;
	IpkgCategory*	m_ExtraPackages;
	IpkgCategory*	m_UpdatesPackages;

	/* actions */
	static void*	TUpdate(void *ptr);
	static void*	TDownload(void *ptr);
	static void*	TInstall(void *ptr);
	static void*	TRemove(void *ptr);
	static void*	TUpgrade(void *ptr);

	/* ipkg callbacks */
	static void		ipkgProgressCallback(const opkg_progress_data_t *progress, void *user_data);
	static void		ipkgPackageCallback(pkg_t *pkg, void *user_data);
	static void		ipkgUpdatesPackageCallback(pkg_t *pkg, void *user_data);
	static void		ipkgMessageCallback(const char* str, void *user_data);
	static void		ipkgErrorCallback(const char* str, void *user_data);

	/* log functions */
	void			sendNotice(const char *message, ...);
	void			sendError(const char *message, ...);

	/* category handling */
	static bool		categorySort(IpkgCategory *a, IpkgCategory *b);

public:
	Ipkg();
	virtual ~Ipkg();

	/* callbacks */
	void setProgressCallback(PRGCBFUNC func, void *clientdata);
	void setNoticeCallback(WRNCBFUNC func, void *clientdata);
	void setErrorCallback(ERRCBFUNC func, void *clientdata);
	void setEndCallback(ENDCBFUNC func, void *clientdata);
	void clearCallbacks();

	/* multithreading */
	void join();

	/* actions */
	void update();
	void download(char *package);
	void install(char *package);
	void remove(char *package);
	void upgrade();

	bool isUpgradeable();

	/* category handling */
	void			categoryInit();
	void			categoryDeinit();
	void			categoryRefresh();
	void			categoryFirst();
	bool			categoryNext();
	IpkgCategory*	categoryGet();
	IpkgCategory*	categoryGetAll();
	IpkgCategory*	categoryGetPlugins();
	IpkgCategory*	categoryGetSettings();
	IpkgCategory*	categoryGetEmulators();
	IpkgCategory*	categoryGetExtra();
	IpkgCategory*	categoryGetUpdates();
	IpkgCategory*	categorySearch(const char *name);
	int				categoryCount();
};

#endif /* IPKG_H_ */
