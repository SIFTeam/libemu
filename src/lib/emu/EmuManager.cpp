#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EmuManager.h"

static pthread_mutex_t mMutex = PTHREAD_MUTEX_INITIALIZER;

EmuManager::EmuManager(const char *path, EmuMessages* messages)
{
	this->m_Messages = messages;
	this->m_Path = path;
	this->m_EmuPath = new char[strlen(this->m_Path) + strlen(EMU_SUBDIR) + 2];
	this->m_CsPath = new char[strlen(this->m_Path) + strlen(CS_SUBDIR) + 2];
	sprintf(this->m_EmuPath, "%s/%s", this->m_Path, EMU_SUBDIR);
	sprintf(this->m_CsPath, "%s/%s", this->m_Path, CS_SUBDIR);
	this->m_CurrentEmu = NULL;
	this->m_CurrentCs = NULL;
	this->m_ExitRequest = false;
	this->m_EcmErrors = 0;
	this->initEcmInfo(&this->m_EcmInfo);
	this->reloadEmu();
	this->reloadCs();
	this->readDefaultEmu();
	this->readDefaultCs();
	pthread_create(&this->m_AsyncThread, NULL, processMonitor, (void*)this);
}

EmuManager::~EmuManager()
{
	this->m_ExitRequest = true;
	pthread_join(this->m_AsyncThread, NULL);

	std::list<Emu*>::iterator it;

	for (it = this->m_Emus.begin(); it != this->m_Emus.end(); it++)
		delete *it;

	delete this->m_EmuPath;
	delete this->m_CsPath;
}

void* EmuManager::processMonitor(void *parent)
{
	EmuManager *pparent = (EmuManager*)parent;
	while(!pparent->m_ExitRequest)
	{
		sleep(3);
		pthread_mutex_lock(&mMutex);
		Emu *emu = NULL;
		if (pparent->m_CurrentEmu != NULL)
			if (!pparent->processExist(pparent->m_CurrentEmu->getProcessToMonitor()))
				emu = pparent->m_CurrentEmu;
		pthread_mutex_unlock(&mMutex);
		if (emu) pparent->startEmu(emu);
		
		pthread_mutex_lock(&mMutex);
		CardReader *cr = NULL;
		if (pparent->m_CurrentCs != NULL)
			if (!pparent->processExist(pparent->m_CurrentCs->getProcessToMonitor()))
				cr = pparent->m_CurrentCs;
				
		pthread_mutex_unlock(&mMutex);
		if (cr) pparent->startCs(cr);
	}
	return NULL;
}

void EmuManager::reloadEmu()
{
	std::list<Emu*> emus = this->m_Emus;
	this->m_Emus.clear();

	DIR *dp;
	struct dirent *dirp;
	if ((dp  = opendir(this->m_EmuPath)) != NULL)
	{
		while ((dirp = readdir(dp)) != NULL)
		{
			if (dirp->d_name[0] == '.')
				continue;

			struct stat buf;
			char *newpath = new char[strlen(this->m_EmuPath) + strlen(dirp->d_name) + 2];
			sprintf(newpath, "%s/%s", this->m_EmuPath, dirp->d_name);

			if (stat(newpath, &buf) == 0 && S_ISDIR(buf.st_mode))	// is a dir
			{
				Emu *emu = new Emu(newpath, this->m_Messages);
				if (emu->isInitialized())
				{
					std::list<Emu*>::iterator it;
					for (it = emus.begin(); it != emus.end(); it++)
					{
						if (**it == emu)
						{
							delete emu;
							emu = *it;
							emus.remove(emu);
							break;
						}
					}

					this->m_Emus.push_back(emu);
				}
				else delete emu;
			}

			delete newpath;
		}
		closedir(dp);
	}
	else
		this->m_Messages->Send(EmuMessages::ERROR, "Error opening directory %s", this->m_EmuPath);
}

void EmuManager::reloadCs()
{
	std::list<CardReader*> crs = this->m_Readers;
	this->m_Readers.clear();

	DIR *dp;
	struct dirent *dirp;
	if ((dp  = opendir(this->m_CsPath)) != NULL)
	{
		while ((dirp = readdir(dp)) != NULL)
		{
			if (dirp->d_name[0] == '.')
				continue;

			struct stat buf;
			char *newpath = new char[strlen(this->m_CsPath) + strlen(dirp->d_name) + 2];
			sprintf(newpath, "%s/%s", this->m_CsPath, dirp->d_name);

			if (stat(newpath, &buf) == 0 && S_ISDIR(buf.st_mode))	// is a dir
			{
				CardReader *cr = new CardReader(newpath, this->m_Messages);
				if (cr->isInitialized())
				{
					std::list<CardReader*>::iterator it;
					for (it = crs.begin(); it != crs.end(); it++)
					{
						if (**it == cr)
						{
							delete cr;
							cr = *it;
							crs.remove(cr);
							break;
						}
					}

					this->m_Readers.push_back(cr);
				}
				else delete cr;
			}

			delete newpath;
		}
		closedir(dp);
	}
	else
		this->m_Messages->Send(EmuMessages::ERROR, "Error opening directory %s", this->m_CsPath);
}

void EmuManager::writeDefaultEmu()
{
	char *confpath = new char[strlen(this->m_Path) + strlen(EMU_DEFAULT) + 2];
	sprintf(confpath, "%s/%s", this->m_Path, EMU_DEFAULT);

	FILE *fd = fopen(confpath, "w");
	fwrite(this->m_DefaultEmu.c_str(), this->m_DefaultEmu.size(), 1, fd);
	fclose(fd);

	delete confpath;
}

void EmuManager::writeDefaultCs()
{
	char *confpath = new char[strlen(this->m_Path) + strlen(CS_DEFAULT) + 2];
	sprintf(confpath, "%s/%s", this->m_Path, CS_DEFAULT);

	FILE *fd = fopen(confpath, "w");
	fwrite(this->m_DefaultCs.c_str(), this->m_DefaultCs.size(), 1, fd);
	fclose(fd);

	delete confpath;
}

void EmuManager::readDefaultEmu()
{
	char *confpath = new char[strlen(this->m_Path) + strlen(EMU_DEFAULT) + 2];
	sprintf(confpath, "%s/%s", this->m_Path, EMU_DEFAULT);

	char content[256];
	memset(content, 0, 256);
	FILE *fd = fopen(confpath, "r");
	if (fd)
	{
		fread(content, 255, 1, fd);
		fclose(fd);
		this->m_DefaultEmu = content;
	}

	delete confpath;
}

void EmuManager::readDefaultCs()
{
	char *confpath = new char[strlen(this->m_Path) + strlen(CS_DEFAULT) + 2];
	sprintf(confpath, "%s/%s", this->m_Path, CS_DEFAULT);

	char content[256];
	memset(content, 0, 256);
	FILE *fd = fopen(confpath, "r");
	if (fd)
	{
		fread(content, 255, 1, fd);
		fclose(fd);
		this->m_DefaultCs = content;
	}

	delete confpath;
}

bool EmuManager::processExist(const char *name)
{
	DIR *dp;
	struct dirent *dirp;
	if ((dp  = opendir("/proc")) != NULL)
	{
		while ((dirp = readdir(dp)) != NULL)
		{
			int pid = atoi(dirp->d_name);
			if (pid)
			{
				FILE *fd;
				char path[256];
				char cmd[256];
				sprintf(path, "/proc/%d/stat", pid);
				if (!(fd = fopen(path, "r"))) continue;
				if (fscanf(fd, "%*d (%255[^)]", cmd) != 1) continue;
				fclose(fd);

				if (strlen(name) == strlen(cmd))
				{
					if (memcmp(name, cmd, strlen(name)) == 0)
					{
						closedir(dp);
						return true;
					}
				}
			}
		}
		closedir(dp);
	}
	return false;
}

bool EmuManager::startEmu(Emu* emu)
{
	if (emu == NULL)
	{
		std::list<Emu*>::iterator it;

		for (it = this->m_Emus.begin(); it != this->m_Emus.end(); it++)
		{
			if (this->m_DefaultEmu == (*it)->getId())
			{
				emu = *it;
				break;
			}
		}
		if (emu == NULL) return false;
	}
	
	if (this->m_CurrentEmu != NULL)
		return false;
	
	pthread_mutex_lock(&mMutex);
	char *script = new char[strlen(emu->getDirectory()) + strlen(emu->getStartScript()) + 2];
	sprintf(script, "%s/%s", emu->getDirectory(), emu->getStartScript());
	setenv("CONFIG_DIRECTORY", emu->getDirectory(), 1);
	system(script);
	delete script;
	sleep(1);
	if (!EmuManager::processExist(emu->getProcessToMonitor()))
	{
		this->m_Messages->Send(EmuMessages::ERROR, "Error starting emulator. Check of binary %s failed", emu->getProcessToMonitor());
		pthread_mutex_unlock(&mMutex);
		return false;
	}

	this->m_CurrentEmu = emu;
	this->m_DefaultEmu = emu->getId();
	this->writeDefaultEmu();
	pthread_mutex_unlock(&mMutex);
	return true;
}

bool EmuManager::startCs(CardReader* cr)
{
	if (cr == NULL)
	{
		std::list<CardReader*>::iterator it;

		for (it = this->m_Readers.begin(); it != this->m_Readers.end(); it++)
		{
			if (this->m_DefaultCs == (*it)->getId())
			{
				cr = *it;
				break;
			}
		}
		if (cr == NULL) return false;
	}
	
	if (this->m_CurrentCs != NULL)
		return false;
	
	pthread_mutex_lock(&mMutex);
	char *script = new char[strlen(cr->getDirectory()) + strlen(cr->getStartScript()) + 2];
	sprintf(script, "%s/%s", cr->getDirectory(), cr->getStartScript());
	setenv("CONFIG_DIRECTORY", cr->getDirectory(), 1);
	system(script);
	delete script;
	sleep(1);
	if (!EmuManager::processExist(cr->getProcessToMonitor()))
	{
		this->m_Messages->Send(EmuMessages::ERROR, "Error starting card server. Check of binary %s failed", cr->getProcessToMonitor());
		pthread_mutex_unlock(&mMutex);
		return false;
	}

	this->m_CurrentCs = cr;
	this->m_DefaultCs = cr->getId();
	this->writeDefaultCs();
	pthread_mutex_unlock(&mMutex);
	return true;
}

void EmuManager::stopEmu(bool writedefault)
{
	if (this->m_CurrentEmu == NULL) return;
	pthread_mutex_lock(&mMutex);
	Emu *emu = this->m_CurrentEmu;
	char *script = new char[strlen(emu->getDirectory()) + strlen(emu->getStopScript()) + 2];
	sprintf(script, "%s/%s", emu->getDirectory(), emu->getStopScript());
	setenv("CONFIG_DIRECTORY", emu->getDirectory(), 1);
	system(script);
	delete script;
	for (int i=0; i<emu->getStopMaxDelayTime(); i++)
	{
		sleep(1);
		if (!EmuManager::processExist(emu->getProcessToMonitor()))
		{
			this->m_CurrentEmu = NULL;
			if (writedefault)
			{
				this->m_DefaultEmu = "";
				this->writeDefaultEmu();
			}
			pthread_mutex_unlock(&mMutex);
			return;
		}
	}

	script = new char[strlen(emu->getDirectory()) + strlen(emu->getStopForcedScript()) + 2];
	sprintf(script, "%s/%s", emu->getDirectory(), emu->getStopForcedScript());
	setenv("CONFIG_DIRECTORY", emu->getDirectory(), 1);
	system(script);
	delete script;
	this->m_CurrentEmu = NULL;
	if (writedefault)
	{
		this->m_DefaultEmu = "";
		this->writeDefaultEmu();
	}
	pthread_mutex_unlock(&mMutex);
}

void EmuManager::stopCs(bool writedefault)
{
	if (this->m_CurrentCs == NULL) return;
	pthread_mutex_lock(&mMutex);
	CardReader *cr = this->m_CurrentCs;
	char *script = new char[strlen(cr->getDirectory()) + strlen(cr->getStopScript()) + 2];
	sprintf(script, "%s/%s", cr->getDirectory(), cr->getStopScript());
	setenv("CONFIG_DIRECTORY", cr->getDirectory(), 1);
	system(script);
	delete script;
	for (int i=0; i<cr->getStopMaxDelayTime(); i++)
	{
		sleep(1);
		if (!EmuManager::processExist(cr->getProcessToMonitor()))
		{
			this->m_CurrentCs = NULL;
			if (writedefault)
			{
				this->m_DefaultCs = "";
				this->writeDefaultCs();
			}
			pthread_mutex_unlock(&mMutex);
			return;
		}
	}

	script = new char[strlen(cr->getDirectory()) + strlen(cr->getStopForcedScript()) + 2];
	sprintf(script, "%s/%s", cr->getDirectory(), cr->getStopForcedScript());
	setenv("CONFIG_DIRECTORY", cr->getDirectory(), 1);
	system(script);
	delete script;
	this->m_CurrentCs = NULL;
	if (writedefault)
	{
		this->m_DefaultCs = "";
		this->writeDefaultCs();
	}
	pthread_mutex_unlock(&mMutex);
}

bool EmuManager::isStartedEmu(Emu* emu)
{
	if (emu == NULL) return this->m_CurrentEmu != NULL;
	return this->m_CurrentEmu == emu;
}

bool EmuManager::isStartedCs(CardReader* cr)
{
	if (cr == NULL) return this->m_CurrentCs != NULL;
	return this->m_CurrentCs == cr;
}

void EmuManager::firstEmu()
{
	this->m_EmusIterator = this->m_Emus.begin();
}

void EmuManager::firstCs()
{
	this->m_ReadersIterator = this->m_Readers.begin();
}

bool EmuManager::nextEmu()
{
	this->m_EmusIterator++;
	if (this->m_EmusIterator == this->m_Emus.end())
		return false;
	return true;
}

bool EmuManager::nextCs()
{
	this->m_ReadersIterator++;
	if (this->m_ReadersIterator == this->m_Readers.end())
		return false;
	return true;
}

Emu* EmuManager::getEmu()
{
	return *this->m_EmusIterator;
}

CardReader* EmuManager::getCs()
{
	return *this->m_ReadersIterator;
}

int EmuManager::countEmu()
{
	return this->m_Emus.size();
}

int EmuManager::countCs()
{
	return this->m_Readers.size();
}

void EmuManager::initEcmInfo(ecm_info_t* ecminfo)
{
	memset(ecminfo->cw0, 0, 8);
	memset(ecminfo->cw1, 0, 8);
	if (this->m_CurrentEmu == NULL) strcpy(ecminfo->name, "No emu");
	else sprintf(ecminfo->name, "%s %s", this->m_CurrentEmu->getName(), this->m_CurrentEmu->getVersion());
	strcpy(ecminfo->system, "unknow");
	strcpy(ecminfo->protocol, "unknow");
	strcpy(ecminfo->address, "unknow");
	ecminfo->caid = 0;
	ecminfo->pid = 0;
	ecminfo->provid = 0;
	ecminfo->time = 0;
	ecminfo->hops = 0;
}

bool EmuManager::parseEcmInfoCCCam(ecm_info_t* ecminfo)
{
	char line[256];
	int sec = 0, msec = 0;
	FILE *fd = fopen("/tmp/ecm.info", "r");
	if (!fd) return false;

	while (fgets(line, 256, fd) != NULL)
	{
		if (sscanf(line, "system: %s)", ecminfo->system) == 1) continue;
		if (sscanf(line, "caid: %hX)", &ecminfo->caid) == 1) continue;
		if (sscanf(line, "provid: %X)", &ecminfo->provid) == 1) continue;
		if (sscanf(line, "pid: %hX)", &ecminfo->pid) == 1) continue;
		if (sscanf(line, "using: %s)", ecminfo->protocol) == 1) continue;
		if (sscanf(line, "address: %s)", ecminfo->address) == 1) continue;
		if (sscanf(line, "hops: %d)", &ecminfo->hops) == 1) continue;
		if (sscanf(line, "ecm time: %d.%d)", &sec, &msec) == 2)
		{
			ecminfo->time = (sec*1000) + msec;
			continue;
		}
	}
	fclose(fd);
	return true;
}

bool EmuManager::parseEcmInfoIncubus(ecm_info_t* ecminfo)
{
	char line[256];
	FILE *fd = fopen("/tmp/ecm.info", "r");
	if (!fd) return false;

	while (fgets(line, 256, fd) != NULL)
	{
		int dwdelay = 0;
		if (sscanf(line, "===== %s ECM on CaID %hX, pid %hX ======", ecminfo->system, &ecminfo->caid, &ecminfo->pid) == 3) continue;
		if (sscanf(line, "cw0 : %hhX %hhX %hhX %hhX %hhX %hhX %hhX %hhX", &ecminfo->cw0[0], &ecminfo->cw0[1], &ecminfo->cw0[2],
			&ecminfo->cw0[3], &ecminfo->cw0[4], &ecminfo->cw0[5], &ecminfo->cw0[6], &ecminfo->cw0[7]) == 8) continue;
		if (sscanf(line, "cw1 : %hhX %hhX %hhX %hhX %hhX %hhX %hhX %hhX", &ecminfo->cw1[0], &ecminfo->cw1[1], &ecminfo->cw1[2],
			&ecminfo->cw1[3], &ecminfo->cw1[4], &ecminfo->cw1[5], &ecminfo->cw1[6], &ecminfo->cw1[7]) == 8) continue;
		if (sscanf(line, "using: %s)", ecminfo->protocol) == 1) continue;
		if (sscanf(line, "address: %s)", ecminfo->address) == 1) continue;
		if (sscanf(line, "prov: %X)", &ecminfo->provid) == 1) continue;
		if (sscanf(line, "hops: %d)", &ecminfo->hops) == 1) continue;
		if (sscanf(line, "dw delay: %d", &dwdelay) == 1)
		{
			ecminfo->time = dwdelay / (1000*1000);
			continue;
		}
	}
	fclose(fd);
	return true;
}

bool EmuManager::parseEcmInfoMgcamd(ecm_info_t* ecminfo)
{
	char line[256];
	char trash[256];
	FILE *fd = fopen("/tmp/ecm.info", "r");
	if (!fd) return false;

	while (fgets(line, 256, fd) != NULL)
	{
		if (sscanf(line, "===== %s ECM on CaID %hX, pid %hX ======", ecminfo->system, &ecminfo->caid, &ecminfo->pid) == 3) continue;
		if (sscanf(line, "cw0: %hhX %hhX %hhX %hhX %hhX %hhX %hhX %hhX", &ecminfo->cw0[0], &ecminfo->cw0[1], &ecminfo->cw0[2],
			&ecminfo->cw0[3], &ecminfo->cw0[4], &ecminfo->cw0[5], &ecminfo->cw0[6], &ecminfo->cw0[7]) == 8) continue;
		if (sscanf(line, "cw1: %hhX %hhX %hhX %hhX %hhX %hhX %hhX %hhX", &ecminfo->cw1[0], &ecminfo->cw1[1], &ecminfo->cw1[2],
			&ecminfo->cw1[3], &ecminfo->cw1[4], &ecminfo->cw1[5], &ecminfo->cw1[6], &ecminfo->cw1[7]) == 8) continue;
		if (sscanf(line, "source: %s (%s at %[^)])", trash, ecminfo->protocol, ecminfo->address) == 3) continue;
		if (sscanf(line, "prov: %X)", &ecminfo->provid) == 1) continue;
		if (sscanf(line, "%d msec)", &ecminfo->time) == 1) continue;
	}
	fclose(fd);
	return true;
}

ecm_info_t*	EmuManager::getEcmInfo()
{
	ecm_info_t ecminfo;
	bool ret = false;
	this->initEcmInfo(&ecminfo);
	if (this->m_CurrentEmu != NULL)
	{
		std::string ecminfoparser = this->m_CurrentEmu->getEcmInfoParser();
		if (ecminfoparser == "none") { /* nothing to do */ }
		else if (ecminfoparser == "cccam") ret = this->parseEcmInfoCCCam(&ecminfo);
		else if (ecminfoparser == "mgcamd") ret = this->parseEcmInfoMgcamd(&ecminfo);
		else if (ecminfoparser == "incubuscamd") ret = this->parseEcmInfoIncubus(&ecminfo);
		else this->m_Messages->Send(EmuMessages::ERROR, "EcmInfo format '%s' is not supported", ecminfoparser.c_str());
	}
	if (ret || (this->m_EcmErrors > 4))
	{
		memcpy(&this->m_EcmInfo, &ecminfo, sizeof(ecm_info_t));
		this->m_EcmErrors = 0;
	}
	else this->m_EcmErrors++;
	
	return &this->m_EcmInfo;
}
