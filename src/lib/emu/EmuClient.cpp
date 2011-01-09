#include <iostream>

#include "EmuClient.h"

#define RSIZE TCP_BUFSIZE_READ

static pthread_mutex_t cMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cMutexB = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cMutexC = PTHREAD_MUTEX_INITIALIZER;

EmuClient::EmuClient(int type, EmuMessages *messages) : TcpSocket(*new SocketHandler())
{
	this->m_Type = type;
	this->m_Messages = messages;
	this->m_CurrentSocketHandler = (SocketHandler*)&this->Handler();
	//this->m_InfoThreadStarted = false;
	this->SetLineProtocol();
	//this->SetDeleteByHandler();
	
	this->m_InfoName = "";
	this->m_InfoSystem = "";
	this->m_InfoCaID = "";
	this->m_InfoPid = "";
	this->m_InfoProtocol = "";
	this->m_InfoAddress = "";
	this->m_InfoProvid = "";
	this->m_InfoTime = "";
	this->m_InfoHops = "";
	this->m_InfoCW0 = "";
	this->m_InfoCW1 = "";
}

EmuClient::~EmuClient()
{
	this->m_CurrentAction = EmuClient::NOTHING;
	// TODO: we need to free SocketHandler
	//if (&this->Handler()) delete &this->Handler();
}

void* EmuClient::pollSocket(void *parent)
{
	EmuClient *pparent = (EmuClient*)parent;
	while (true)
	{
		pthread_mutex_lock(&cMutexB);
		if (!pparent->m_CurrentSocketHandler->GetCount() || !pparent->m_Connected || !pparent->IsConnected())
		{
			pthread_mutex_unlock(&cMutexB);
			break;
		}
		pthread_mutex_unlock(&cMutexB);
		pparent->m_CurrentSocketHandler->Select(1, 0);
	}
	return NULL;
}

void* EmuClient::autoGetInfo(void *parent)
{
	EmuClient *pparent = (EmuClient*)parent;
	while (true)
	{
		pthread_mutex_lock(&cMutexB);
		if (!pparent->m_Connected || !pparent->IsConnected())
		{
			pthread_mutex_unlock(&cMutexB);
			break;
		}
		if (pparent->m_ActionInProgress)
		{
			pthread_mutex_unlock(&cMutexB);
			sleep(1);
			continue;
		}
		pthread_mutex_unlock(&cMutexB);
		pparent->sendEcmInfo();
		sleep(1);
	}
	return NULL;
}

bool EmuClient::connect()
{
	this->Open("127.0.0.1", CONNECT_PORT);
	this->m_CurrentSocketHandler->Add(this);
	this->m_CurrentSocketHandler->Select(1, 0);
	if (this->m_CurrentSocketHandler->GetCount())
	{
		pthread_mutex_lock(&cMutexB);
		this->m_Connected = true;
		pthread_mutex_unlock(&cMutexB);
		pthread_create(&this->m_AsyncThread, NULL, pollSocket, (void*)this);
		if (this->m_Type == EmuClient::EMU)
			pthread_create(&this->m_InfoThread, NULL, autoGetInfo, (void*)this);
		return true;
	}
	pthread_mutex_lock(&cMutexB);
	this->m_Connected = false;
	pthread_mutex_unlock(&cMutexB);
	return false;
}

void EmuClient::disconnect()
{
	pthread_mutex_lock(&cMutexB);
	this->m_Connected = false;
	pthread_mutex_unlock(&cMutexB);
	pthread_join(this->m_AsyncThread, NULL);
	if (this->m_Type == EmuClient::EMU)
		pthread_join(this->m_InfoThread, NULL);
	//this->m_CurrentSocketHandler->Remove(this);
}

bool EmuClient::isConnected()
{
	return this->m_Connected;
}
//void EmuClient::startAutoGetInfo()
//{
//	this->m_InfoThreadStarted = true;
//	pthread_create(&this->m_InfoThread, NULL, autoGetInfo, (void*)this);
//}

// 30 seconds timeout
#define TIMEOUT 300
void EmuClient::waitAction()
{
	int timeout = 0;
	while (true)
	{
		pthread_mutex_lock(&cMutexB);
		if (!this->m_ActionInProgress || !this->m_Connected || !this->IsConnected() || !(timeout < TIMEOUT))
		{
			pthread_mutex_unlock(&cMutexB);
			break;
		}
		pthread_mutex_unlock(&cMutexB);
		
		usleep(100000);
		timeout++;
	}
	if (timeout == TIMEOUT)
		m_Messages->Send(EmuMessages::ERROR, "Action timed out");
}

void EmuClient::sendList()
{
	pthread_mutex_lock(&cMutexC);
	pthread_mutex_lock(&cMutexB);
	this->m_ActionInProgress = true;
	this->m_CurrentAction = EmuClient::LIST;
	this->m_EmuList.clear();
	pthread_mutex_unlock(&cMutexB);
	
	switch (this->m_Type)
	{
	case EmuClient::EMU:
		this->Sendf("emu list\n");
		break;

	case EmuClient::CS:
		this->Sendf("cs list\n");
		break;

	default:
		pthread_mutex_unlock(&cMutexC);
		return;
	}
	
	this->waitAction();
	pthread_mutex_unlock(&cMutexC);
}

bool EmuClient::sendStart(const char* emuid)
{
	pthread_mutex_lock(&cMutexC);
	pthread_mutex_lock(&cMutexB);
	this->m_ActionInProgress = true;
	this->m_CurrentAction = EmuClient::START;
	pthread_mutex_unlock(&cMutexB);
	
	switch (this->m_Type)
	{
	case EmuClient::EMU:
		if (emuid == NULL)
			this->Sendf("emu start\n");
		else
			this->Sendf("emu start %s\n", emuid);
		break;

	case EmuClient::CS:
		if (emuid == NULL)
			this->Sendf("cs start\n");
		else
			this->Sendf("cs start %s\n", emuid);
		break;

	default:
		pthread_mutex_unlock(&cMutexC);
		return false;
	}

	this->waitAction();
	bool ret = this->m_LastStatus;
	pthread_mutex_unlock(&cMutexC);
	return ret;
}

bool EmuClient::sendRestart()
{
	pthread_mutex_lock(&cMutexC);
	pthread_mutex_lock(&cMutexB);
	this->m_ActionInProgress = true;
	this->m_CurrentAction = EmuClient::RESTART;
	pthread_mutex_unlock(&cMutexB);
	
	switch (this->m_Type)
	{
	case EmuClient::EMU:
		this->Sendf("emu restart\n");
		break;

	case EmuClient::CS:
		this->Sendf("cs restart\n");
		break;

	default:
		pthread_mutex_unlock(&cMutexC);
		return false;
	}
	
	this->waitAction();
	bool ret = this->m_LastStatus;
	pthread_mutex_unlock(&cMutexC);
	return ret;
}

void EmuClient::sendEcmInfo()
{
	pthread_mutex_lock(&cMutexC);
	pthread_mutex_lock(&cMutexB);
	this->m_ActionInProgress = true;
	this->m_CurrentAction = EmuClient::ECMINFO;
	this->m_InfoName = "";
	this->m_InfoSystem = "";
	this->m_InfoCaID = "";
	this->m_InfoPid = "";
	this->m_InfoProtocol = "";
	this->m_InfoAddress = "";
	this->m_InfoProvid = "";
	this->m_InfoTime = "";
	this->m_InfoHops = "";
	this->m_InfoCW0 = "";
	this->m_InfoCW1 = "";
	pthread_mutex_unlock(&cMutexB);
	
	switch (this->m_Type)
	{
	case EmuClient::EMU:
		this->Sendf("emu ecminfo\n");
		break;

	default:
		pthread_mutex_unlock(&cMutexC);
		return;
	}

	this->waitAction();
	pthread_mutex_unlock(&cMutexC);
}

bool EmuClient::sendStop()
{
	pthread_mutex_lock(&cMutexC);
	pthread_mutex_lock(&cMutexB);
	this->m_ActionInProgress = true;
	this->m_CurrentAction = EmuClient::STOP;
	pthread_mutex_unlock(&cMutexB);
	
	switch (this->m_Type)
	{
	case EmuClient::EMU:
		this->Sendf("emu stop\n");
		break;

	case EmuClient::CS:
		this->Sendf("cs stop\n");
		break;

	default:
		pthread_mutex_unlock(&cMutexC);
		return false;
	}

	this->waitAction();
	bool ret = this->m_LastStatus;
	pthread_mutex_unlock(&cMutexC);
	return ret;
}

int EmuClient::splitString(const std::string &input, const std::string &delimiter, std::vector<std::string> &results, bool includeEmpties)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.size();
	int isize = (int)input.size();

	if ((isize == 0) || (sizeS2 == 0))
		return 0;

	std::vector<int> positions;

	newPos = input.find (delimiter, 0);

	if (newPos < 0)
		return 0;

	int numFound = 0;

	while (newPos >= iPos)
	{
		numFound++;
		positions.push_back(newPos);
		iPos = newPos;
		newPos = input.find (delimiter, iPos+sizeS2);
	}

	if (numFound == 0)
		return 0;

	for (unsigned int i=0; i <= positions.size(); ++i)
	{
		std::string s("");
		if (i == 0) s = input.substr(i, positions[i]);

		int offset = positions[i-1] + sizeS2;
		if (offset < isize)
		{
			if (i == positions.size()) s = input.substr(offset);
			else if (i > 0) s = input.substr(positions[i-1] + sizeS2, positions[i] - positions[i-1] - sizeS2);
		}
		if (includeEmpties || (s.size() > 0)) results.push_back(s);
	}

	return numFound;
}

void EmuClient::parseList(std::string line)
{
	this->m_Messages->Send(EmuMessages::DEBUG, "Parse list: %s", line.c_str());
	if (line == "!!ended")
	{
		pthread_mutex_lock(&cMutexB);
		this->m_CurrentAction = EmuClient::NOTHING;
		this->m_ActionInProgress = false;
		pthread_mutex_unlock(&cMutexB);
		this->m_Messages->Send(EmuMessages::DEBUG, "Parsing ended");
	}
	else
	{
		std::vector<std::string> res;
		pthread_mutex_lock(&cMutexB);
		EmuClient::splitString(line, "|", res);
		pthread_mutex_unlock(&cMutexB);
		if (res.size() != 5)
			m_Messages->Send(EmuMessages::ERROR, "Corrupted server reply: %s", line.c_str());
		else
		{
			infoemu_t info;
			info.m_Id = res[0];
			info.m_Name = res[1];
			info.m_Version = res[2];
			info.m_Description = res[3];
			info.m_IsStarted = res[4] != "0";
			pthread_mutex_lock(&cMutexB);
			this->m_EmuList.push_back(info);
			pthread_mutex_unlock(&cMutexB);
		}
	}
}

void EmuClient::parseEcmInfo(std::string line)
{
	if (line == "!!ended")
	{
		pthread_mutex_lock(&cMutexB);
		this->m_CurrentAction = EmuClient::NOTHING;
		this->m_ActionInProgress = false;
		pthread_mutex_unlock(&cMutexB);
	}
	else
	{
		pthread_mutex_lock(&cMutexB);
		if (line.substr(0, 9) == "!!emuname") this->m_InfoName = line.substr(10);
		if (line.substr(0, 8) == "!!system") this->m_InfoSystem = line.substr(9);
		if (line.substr(0, 6) == "!!caid") this->m_InfoCaID = line.substr(7);
		if (line.substr(0, 5) == "!!pid") this->m_InfoPid = line.substr(6);
		if (line.substr(0, 10) == "!!protocol") this->m_InfoProtocol = line.substr(11);
		if (line.substr(0, 9) == "!!address") this->m_InfoAddress = line.substr(10);
		if (line.substr(0, 8) == "!!provid") this->m_InfoProvid = line.substr(9);
		if (line.substr(0, 6) == "!!time") this->m_InfoTime = line.substr(7);
		if (line.substr(0, 6) == "!!hops") this->m_InfoHops = line.substr(7);
		if (line.substr(0, 5) == "!!cw0") this->m_InfoCW0 = line.substr(6);
		if (line.substr(0, 5) == "!!cw1") this->m_InfoCW1 = line.substr(6);
		pthread_mutex_unlock(&cMutexB);
	}
}

void EmuClient::parseCommonAction(std::string line)
{
	if (line == "!!ok")
	{
		pthread_mutex_lock(&cMutexB);
		this->m_CurrentAction = EmuClient::NOTHING;
		this->m_ActionInProgress = false;
		this->m_LastStatus = true;
		pthread_mutex_unlock(&cMutexB);
	}
	else if (line.substr(0, 5) == "!!err")
	{
		m_Messages->Send(EmuMessages::ERROR, line.substr(6).c_str());
		pthread_mutex_lock(&cMutexB);
		this->m_CurrentAction = EmuClient::NOTHING;
		this->m_ActionInProgress = false;
		this->m_LastStatus = false;
		pthread_mutex_unlock(&cMutexB);
	}
}

void EmuClient::emuFirst()
{
	pthread_mutex_lock(&cMutexB);
	this->m_EmuListIterator = this->m_EmuList.begin();
	pthread_mutex_unlock(&cMutexB);
}

bool EmuClient::emuNext()
{
	pthread_mutex_lock(&cMutexB);
	this->m_EmuListIterator++;
	if (this->m_EmuListIterator == this->m_EmuList.end())
	{
		pthread_mutex_unlock(&cMutexB);
		return false;
	}
	pthread_mutex_unlock(&cMutexB);
	return true;
}

bool EmuClient::emuGetIsStarted()
{
	return this->m_EmuListIterator->m_IsStarted;
}

const char* EmuClient::emuGetId()
{
	return this->m_EmuListIterator->m_Id.c_str();
}

const char* EmuClient::emuGetName()
{
	return this->m_EmuListIterator->m_Name.c_str();
}

const char* EmuClient::emuGetVersion()
{
	return this->m_EmuListIterator->m_Version.c_str();
}

const char* EmuClient::emuGetDescription()
{
	return this->m_EmuListIterator->m_Description.c_str();
}

int EmuClient::emuCount()
{
	return this->m_EmuList.size();
}

void EmuClient::OnLine(const std::string& line)
{
	pthread_mutex_lock(&cMutex);
	m_Messages->Send(EmuMessages::DEBUG, "Reply received: %s", line.c_str());
	switch(this->m_CurrentAction)
	{
	case EmuClient::ECMINFO:
		this->parseEcmInfo(line);
		break;
	case EmuClient::LIST:
		this->parseList(line);
		break;
	case EmuClient::START:
	case EmuClient::STOP:
	case EmuClient::RESTART:
		this->parseCommonAction(line);
		break;
	}
	pthread_mutex_unlock(&cMutex);
}

void EmuClient::OnConnect()
{
	pthread_mutex_lock(&cMutex);
	m_Messages->Send(EmuMessages::DEBUG, "Connected");
	pthread_mutex_unlock(&cMutex);
}

const char* EmuClient::getInfoName()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoName;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}
const char* EmuClient::getInfoSystem()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoSystem;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoCaID()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoCaID;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoPid()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoPid;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoProtocol()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoProtocol;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoAddress()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoAddress;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoProvID()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoProvid;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoTime()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoTime;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoHops()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoHops;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoCW0()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoCW0;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}

const char* EmuClient::getInfoCW1()
{
	std::string tmp;
	pthread_mutex_lock(&cMutexB);
	tmp = this->m_InfoCW1;
	pthread_mutex_unlock(&cMutexB);
	return tmp.c_str();
}
