#include <iostream>

#include "lib/emu/EmuClient.h"

int main(int argc, char **argv)
{
	EmuMessages *messages = new EmuMessages();
	bool showopt = true;
	int action = EmuClient::NOTHING;
	int emunumber = 0;

	if (argc > 1)
	{
		std::string cmd = argv[1];
		if (cmd == "list")
		{
			action = EmuClient::LIST;
			showopt = false;
		}
		else if (cmd == "start")
		{
			if (argc == 3)
			{
				emunumber = strtol(argv[2], NULL, 10);
				if (emunumber > 0)
				{
					action = EmuClient::START;
					showopt = false;
				}
			}
		}
		else if (cmd == "stop")
		{
			action = EmuClient::STOP;
			showopt = false;
		}
		else if (cmd == "restart")
		{
			action = EmuClient::RESTART;
			showopt = false;
		}
		else if (cmd == "ecminfo")
		{
			action = EmuClient::ECMINFO;
			showopt = false;
		}
	}

	if (showopt)
	{
		messages->Send(EmuMessages::INFO, "SIFTeam emu manager utility");
		messages->Send(EmuMessages::INFO, "");
		messages->Send(EmuMessages::INFO, "usage:");
		messages->Send(EmuMessages::INFO, "%s list", argv[0]);
		messages->Send(EmuMessages::INFO, "%s start [emu_number]", argv[0]);
		messages->Send(EmuMessages::INFO, "%s stop", argv[0]);
		messages->Send(EmuMessages::INFO, "%s restart", argv[0]);
		messages->Send(EmuMessages::INFO, "%s ecminfo", argv[0]);

		delete messages;
		return 0;
	}

	EmuClient *p = new EmuClient(EmuClient::EMU, messages);

	if (!p->connect()) messages->Send(EmuMessages::ERROR, "Cannot connect to emu daemon");
	else
	{
		switch(action)
		{
		case EmuClient::LIST:
			p->sendList();
			if (p->emuCount() > 0)
			{
				int i = 1;
				messages->Send(EmuMessages::INFO, "  | Status  | Name                           | Version  | Description");
				p->emuFirst();
				do
				{
					std::string name = p->emuGetName();
					std::string version = p->emuGetVersion();
					std::string description = p->emuGetDescription();
					name.insert(name.end(), 30 - name.size(), ' ');
					version.insert(version.end(), 8 - version.size(), ' ');
					description.insert(description.end(), 60 - description.size(), ' ');
					if (p->emuGetIsStarted())
						messages->Send(EmuMessages::INFO, "%d | started | %s | %s | %s", i, name.c_str(), version.c_str(), description.c_str());
					else
						messages->Send(EmuMessages::INFO, "%d |         | %s | %s | %s", i, name.c_str(), version.c_str(), description.c_str());
					i++;
				}
				while (p->emuNext());
			}
			else messages->Send(EmuMessages::ERROR, "No emus found");
			break;

		case EmuClient::START:
			p->sendList();
			if (p->emuCount() >= emunumber)
			{
				p->emuFirst();
				for (int i=0; i<emunumber-1; i++) p->emuNext();
				p->sendStart(p->emuGetId());
			}
			else messages->Send(EmuMessages::ERROR, "Selected emu doesn't exist");
			break;

		case EmuClient::STOP:
			p->sendStop();
			break;

		case EmuClient::RESTART:
			p->sendRestart();
			break;
		case EmuClient::ECMINFO:
			p->sendEcmInfo();
			messages->Send(EmuMessages::INFO, "Name: %s", p->getInfoName());
			messages->Send(EmuMessages::INFO, "System: %s", p->getInfoSystem());
			messages->Send(EmuMessages::INFO, "CaID: %s", p->getInfoCaID());
			messages->Send(EmuMessages::INFO, "Pid: %s", p->getInfoPid());
			messages->Send(EmuMessages::INFO, "Protocol: %s", p->getInfoProtocol());
			messages->Send(EmuMessages::INFO, "Address: %s", p->getInfoAddress());
			messages->Send(EmuMessages::INFO, "ProvID: %s", p->getInfoProvID());
			messages->Send(EmuMessages::INFO, "Time (ms): %s", p->getInfoTime());
			messages->Send(EmuMessages::INFO, "Hops: %s", p->getInfoHops());
			messages->Send(EmuMessages::INFO, "CW0: %s", p->getInfoCW0());
			messages->Send(EmuMessages::INFO, "CW1: %s", p->getInfoCW1());
			
			break;
		}
		p->disconnect();
	}

	delete messages;
	return 0;
}
