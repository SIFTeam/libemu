#ifndef EMUMANAGER_H_
#define EMUMANAGER_H_

#include "Emu.h"
#include "CardReader.h"
#include "EmuMessages.h"

#include <list>

#define EMU_DEFAULT "emu.default"
#define EMU_SUBDIR "emu"
#define CS_DEFAULT "cs.default"
#define CS_SUBDIR "cs"

typedef struct ecm_info_s
{
	char			name[256];
	char			system[256];
	unsigned short	caid;
	unsigned short	pid;
	unsigned char	cw0[8];
	unsigned char	cw1[8];
	char			protocol[256];
	char			address[256];
	unsigned int	provid;
	unsigned int	time;
	unsigned int	hops;
} ecm_info_t;

class EmuManager
{
private:
	EmuMessages*	m_Messages;
	const char*		m_Path;
	char*			m_EmuPath;
	char*			m_CsPath;

	std::list<Emu*>				m_Emus;
	std::list<Emu*>::iterator	m_EmusIterator;
	std::list<CardReader*>				m_Readers;
	std::list<CardReader*>::iterator	m_ReadersIterator;

	std::string		m_DefaultEmu;
	Emu*			m_CurrentEmu;
	std::string		m_DefaultCs;
	CardReader*		m_CurrentCs;
	
	ecm_info_t		m_EcmInfo;

	volatile bool	m_ExitRequest;
	pthread_t		m_AsyncThread;

	static void* processMonitor(void *parent);
	bool processExist(const char *name);

	void writeDefaultEmu();
	void writeDefaultCs();
	void readDefaultEmu();
	void readDefaultCs();

	int		m_EcmErrors;
	void	initEcmInfo(ecm_info_t *ecminfo);
	bool	parseEcmInfoIncubus(ecm_info_t *ecminfo);
	bool	parseEcmInfoMgcamd(ecm_info_t *ecminfo);
	bool	parseEcmInfoCCCam(ecm_info_t *ecminfo);

public:
	EmuManager(const char *path, EmuMessages *messages);
	virtual ~EmuManager();

	void	reloadEmu();
	void	reloadCs();

	bool 	startEmu(Emu* emu = NULL);
	bool 	startCs(CardReader* cr = NULL);
	void	stopEmu(bool writedefault = true);
	void	stopCs(bool writedefault = true);

	bool	isStartedEmu(Emu* emu = NULL);
	bool	isStartedCs(CardReader* cr = NULL);

	void	firstEmu();
	bool	nextEmu();
	Emu*	getEmu();
	int		countEmu();

	void		firstCs();
	bool		nextCs();
	CardReader*	getCs();
	int			countCs();
	
	ecm_info_t*	getEcmInfo();
};

#endif /* EMUMANAGER_H_ */
