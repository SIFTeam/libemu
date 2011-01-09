#ifndef EMU_H_
#define EMU_H_

#define EMU_CONFIG "emulator.conf"

#include "EmuMessages.h"

class Emu
{
private:
	EmuMessages*	m_Messages;
	bool			m_Initialized;

	std::string		m_Dir;
	std::string		m_Id;

	std::string		m_Name;
	std::string		m_Version;
	std::string		m_Description;
	std::string		m_ProcessToMonitor;
	std::string		m_StartScript;
	std::string		m_StopScript;
	std::string		m_StopForcedScript;
	std::string		m_EcmInfoParser;

	int				m_StopMaxDelayTime;

public:
	Emu(const char *path, EmuMessages *messages);
	virtual ~Emu();

	bool	operator==(Emu *emu);
	bool	operator==(const char *emuid);

	bool	isInitialized();

	const char*	getDirectory();
	const char*	getId();
	const char*	getName();
	const char*	getVersion();
	const char*	getDescription();
	const char*	getProcessToMonitor();
	const char*	getStartScript();
	const char*	getStopScript();
	const char*	getStopForcedScript();
	int			getStopMaxDelayTime();
	const char*	getEcmInfoParser();
};

#endif /* EMU_H_ */
