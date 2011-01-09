#include <iostream>
#include <SocketHandler.h>
#include <ListenSocket.h>

#include <signal.h>
#include <sys/stat.h>

#include "lib/emu/EmuManager.h"
#include "lib/emu/EmuServer.h"

#define LISTEN_PORT 4626

#ifndef EMUD_CONF_PATH
#define EMUD_CONF_PATH "/etc/emud"
#endif

static volatile bool kill_request;

void sigint_handler(int sig) { kill_request = true; }

int main(int argc, char **argv)
{
	static struct sigaction act;

	kill_request = false;
	act.sa_handler = sigint_handler;
	act.sa_flags = 0;
	sigfillset (&(act.sa_mask));      /* create full set of signals */

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGKILL, &act, NULL);

	EmuMessages *messages = new EmuMessages();
	EmuManager *manager = new EmuManager(EMUD_CONF_PATH, messages);

	EmuServer::setManager(manager);
	EmuServer::setMessages(messages);

	ipaddr_t bindip = 0;
	SocketHandler h;
	ListenSocket<EmuServer> l(h);

	bindip |= 127;
	bindip |= 1 << 24;
	if (l.Bind(bindip, (port_t)LISTEN_PORT))
		return false;

	h.Add(&l);
	h.Select(1,0);
	while (!kill_request)
	{
		h.Select(1,0);
	}

	manager->stopEmu(false);
	manager->stopCs(false);

	delete manager;
	delete messages;
	return 0;
}
