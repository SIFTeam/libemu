#ifndef EMUCLIENT_H_
#define EMUCLIENT_H_

#include <TcpSocket.h>
#include <ISocketHandler.h>
#include <SocketHandler.h>

#include "EmuMessages.h"
#include "EmuManager.h"

#define CONNECT_PORT 4626

class EmuClient : public TcpSocket
{
private:
	typedef struct infoemu_s
	{
		std::string m_Id;
		std::string m_Name;
		std::string m_Version;
		std::string m_Description;
		bool m_IsStarted;
	} infoemu_t;

	bool			m_ActionInProgress;
	int				m_CurrentAction;
	int				m_Type;
	SocketHandler*	m_CurrentSocketHandler;
	bool			m_LastStatus;

	EmuMessages*	m_Messages;

	bool			m_Connected;
	static void*	pollSocket(void *parent);
	pthread_t		m_AsyncThread;
	static void*	autoGetInfo(void *parent);
	pthread_t		m_InfoThread;
	//bool			m_InfoThreadStarted;

	std::list<infoemu_t>			m_EmuList;
	std::list<infoemu_t>::iterator	m_EmuListIterator;
	
	std::string	m_InfoName;
	std::string	m_InfoSystem;
	std::string	m_InfoCaID;
	std::string	m_InfoPid;
	std::string	m_InfoProtocol;
	std::string	m_InfoAddress;
	std::string	m_InfoProvid;
	std::string	m_InfoTime;
	std::string	m_InfoHops;
	std::string	m_InfoCW0;
	std::string	m_InfoCW1;

	static int	splitString(const std::string &input, const std::string &delimiter, std::vector<std::string> &results, bool includeEmpties = true);

	void	parseList(std::string line);
	void	parseEcmInfo(std::string line);
	void	parseCommonAction(std::string line);
	
	void	waitAction();

public:
	enum { NOTHING = 0, LIST, START, STOP, RESTART, ECMINFO };
	enum { EMU, CS };

	EmuClient(int type, EmuMessages *messages);
	virtual ~EmuClient();

	bool connect();
	void disconnect();
	bool isConnected();
	
	//void startAutoGetInfo();

	void sendList();
	bool sendStart(const char* emuid = NULL);
	bool sendRestart();
	bool sendStop();
	void sendEcmInfo();

	/* emu list management */
	void		emuFirst();
	bool		emuNext();
	bool		emuGetIsStarted();
	const char*	emuGetId();
	const char*	emuGetName();
	const char*	emuGetVersion();
	const char*	emuGetDescription();
	int			emuCount();
	
	/* ecminfo management */
	const char* getInfoName();
	const char* getInfoSystem();
	const char* getInfoCaID();
	const char* getInfoPid();
	const char* getInfoProtocol();
	const char* getInfoAddress();
	const char* getInfoProvID();
	const char* getInfoTime();
	const char* getInfoHops();
	const char* getInfoCW0();
	const char* getInfoCW1();

	/* sockets callbacks */
	void OnLine(const std::string& line);
	void OnConnect();
	/*
	void OnReconnect();
	bool OnConnectRetry();
	void OnConnectFailed();
	void OnDisconnect();
	void OnConnectTimeout();
	void OnTimeout();
	*/
};

#endif /* EMUCLIENT_H_ */
