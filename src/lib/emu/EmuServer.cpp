#include <iostream>
#include <algorithm>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysctl.h>

#include "EmuServer.h"

#define RSIZE TCP_BUFSIZE_READ

pthread_mutex_t sMutex = PTHREAD_MUTEX_INITIALIZER;

static EmuMessages*	sMessages;
static EmuManager*	sManager;

EmuServer::EmuServer(ISocketHandler& h) : TcpSocket(h)
{
	sMessages->Send(EmuMessages::DEBUG, "Start socket listening");
	SetLineProtocol();
}

EmuServer::~EmuServer()
{
	sMessages->Send(EmuMessages::DEBUG, "Connection closed");
}

void EmuServer::setMessages(EmuMessages *messages)
{
	sMessages = messages;
}

void EmuServer::setManager(EmuManager *manager)
{
	sManager = manager;
}

void EmuServer::cmdListEmu()
{
	sManager->reloadEmu();
	if (sManager->countEmu() > 0)
	{
		sManager->firstEmu();
		do
		{
			bool started = sManager->isStartedEmu(sManager->getEmu());
			this->Sendf("%s|%s|%s|%s|%d\n", sManager->getEmu()->getId(), sManager->getEmu()->getName(), sManager->getEmu()->getVersion(), sManager->getEmu()->getDescription(), started);
		}
		while (sManager->nextEmu());
	}
	this->Sendf("!!ended\n");
}

void EmuServer::cmdListCs()
{
	sManager->reloadCs();
	if (sManager->countCs() > 0)
	{
		sManager->firstCs();
		do
		{
			bool started = sManager->isStartedCs(sManager->getCs());
			this->Sendf("%s|%s|%s|%s|%d\n", sManager->getCs()->getId(), sManager->getCs()->getName(), sManager->getCs()->getVersion(), sManager->getCs()->getDescription(), started);
		}
		while (sManager->nextCs());
	}
	this->Sendf("!!ended\n");
}

void EmuServer::cmdStartEmu(std::string camid)
{
	Emu* selected = NULL;
	if (sManager->countEmu() > 0)
	{
		sManager->firstEmu();
		do
		{
			if (*sManager->getEmu() == camid.c_str())
			{
				selected = sManager->getEmu();
				break;
			}
		}
		while (sManager->nextEmu());
	}

	if (selected)
	{
		sManager->stopEmu();
		if (sManager->startEmu(selected)) this->Sendf("!!ok\n");
		else this->Sendf("!!err Cannot start emulator. Please check your configuration\n");
		return;
	}
	this->Sendf("!!err Emulator not found\n");
}

void EmuServer::cmdStartDefaultEmu()
{
	if (sManager->startEmu()) this->Sendf("!!ok\n");
	else this->Sendf("!!err Cannot start emulator. Please check your configuration\n");
}

void EmuServer::cmdStartCs(std::string csid)
{
	CardReader* selected = NULL;
	if (sManager->countCs() > 0)
	{
		sManager->firstCs();
		do
		{
			if (*sManager->getCs() == csid.c_str())
			{
				selected = sManager->getCs();
				break;
			}
		}
		while (sManager->nextCs());
	}

	if (selected)
	{
		sManager->stopCs();
		if (sManager->startCs(selected)) this->Sendf("!!ok\n");
		else this->Sendf("!!err Cannot start card server. Please check your configuration\n");
		return;
	}
	this->Sendf("!!err Card server not found\n");
}

void EmuServer::cmdStartDefaultCs()
{
	if (sManager->startCs()) this->Sendf("!!ok\n");
	else this->Sendf("!!err Cannot start card server. Please check your configuration\n");
}

void EmuServer::cmdRestartDefaultEmu()
{
	sManager->stopEmu(false);
	if (sManager->startEmu()) this->Sendf("!!ok\n");
	else this->Sendf("!!err Cannot start emulator. Please check your configuration\n");
}

void EmuServer::cmdRestartDefaultCs()
{
	sManager->stopCs(false);
	if (sManager->startCs()) this->Sendf("!!ok\n");
	else this->Sendf("!!err Cannot start card server. Please check your configuration\n");
}

void EmuServer::cmdStopEmu()
{
	sManager->stopEmu();
	this->Sendf("!!ok\n");
}

void EmuServer::cmdStopCs()
{
	sManager->stopCs();
	this->Sendf("!!ok\n");
}

void EmuServer::cmdGetEcm()
{
	ecm_info_t *ecminfo = sManager->getEcmInfo();
	this->Sendf("!!emuname %s\n", ecminfo->name);
	this->Sendf("!!system %s\n", ecminfo->system);
	this->Sendf("!!caid 0x%X\n", ecminfo->caid);
	this->Sendf("!!pid 0x%X\n", ecminfo->pid);
	this->Sendf("!!protocol %s\n", ecminfo->protocol);
	this->Sendf("!!address %s\n", ecminfo->address);
	this->Sendf("!!provid 0x%X\n", ecminfo->provid);
	this->Sendf("!!time %d\n", ecminfo->time);
	this->Sendf("!!hops %d\n", ecminfo->hops);
	this->Sendf("!!cw0 %X %X %X %X %X %X %X %X\n", ecminfo->cw0[0], ecminfo->cw0[1], ecminfo->cw0[2], ecminfo->cw0[3], 
											ecminfo->cw0[4], ecminfo->cw0[5], ecminfo->cw0[6], ecminfo->cw0[7]);
	this->Sendf("!!cw1 %X %X %X %X %X %X %X %X\n", ecminfo->cw1[0], ecminfo->cw1[1], ecminfo->cw1[2], ecminfo->cw1[3], 
											ecminfo->cw1[4], ecminfo->cw1[5], ecminfo->cw1[6], ecminfo->cw1[7]);
	this->Sendf("!!ended\n");
}

void EmuServer::OnLine(const std::string& line)
{
	pthread_mutex_lock(&sMutex);
	std::string cmd = line;
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
	sMessages->Send(EmuMessages::DEBUG, "Command received: %s", cmd.c_str());
	if (cmd == "quit") this->SetCloseAndDelete();
	else if (cmd == "emu list") this->cmdListEmu();
	else if (cmd == "cs list") this->cmdListCs();
	else if (cmd == "emu start") this->cmdStartDefaultEmu();
	else if (cmd == "cs start") this->cmdStartDefaultCs();
	else if (cmd.substr(0, 9) == "emu start") this->cmdStartEmu(line.substr(10));
	else if (cmd.substr(0, 8) == "cs start") this->cmdStartCs(line.substr(9));
	else if (cmd == "emu restart") this->cmdRestartDefaultEmu();
	else if (cmd == "cs restart") this->cmdRestartDefaultCs();
	else if (cmd == "emu stop") this->cmdStopEmu();
	else if (cmd == "cs stop") this->cmdStopCs();
	else if (cmd == "emu ecminfo") this->cmdGetEcm();
	pthread_mutex_unlock(&sMutex);
}

void EmuServer::OnAccept()
{
	pthread_mutex_lock(&sMutex);
	sMessages->Send(EmuMessages::DEBUG, "Connection accepted");
	pthread_mutex_unlock(&sMutex);
}
