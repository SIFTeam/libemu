#ifndef EMUSERVER_H_
#define EMUSERVER_H_

#include <TcpSocket.h>
#include <ISocketHandler.h>

#include "EmuMessages.h"
#include "EmuManager.h"

class EmuServer : public TcpSocket
{
private:

public:
	EmuServer(ISocketHandler&);
	virtual ~EmuServer();

	static void setMessages(EmuMessages *messages);
	static void setManager(EmuManager *manager);

	void cmdListEmu();
	void cmdListCs();
	void cmdStartEmu(std::string camid);
	void cmdStartCs(std::string csid);
	void cmdStartDefaultEmu();
	void cmdStartDefaultCs();
	void cmdRestartDefaultEmu();
	void cmdRestartDefaultCs();
	void cmdStopEmu();
	void cmdStopCs();
	void cmdGetEcm();

	/* sockets callbacks */
	void OnLine(const std::string& line);
	void OnAccept();
};

#endif /* EMUSERVER_H_ */
