#ifndef CARDREADER_H_
#define CARDREADER_H_

#define CR_CONFIG "cardserver.conf"

#include "EmuMessages.h"

class CardReader
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

	int				m_StopMaxDelayTime;

public:
	CardReader(const char *path, EmuMessages *messages);
	virtual ~CardReader();

	bool	operator==(CardReader *cr);
	bool	operator==(const char *crid);

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
};

#endif /* CARDREADER_H_ */
