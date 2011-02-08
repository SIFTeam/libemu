#include <iostream>
#include <string.h>

#include "Emu.h"
#include "../3rd/ConfigFile.h"

Emu::Emu(const char *path, EmuMessages *messages)
{
	this->m_Messages = messages;
	this->m_Initialized = false;

	char *confpath = new char[strlen(path) + strlen(EMU_CONFIG) + 2];
	sprintf(confpath, "%s/%s", path, EMU_CONFIG);
	try
	{
		ConfigFile config(confpath);
		config.readInto(this->m_Name, "name");
		config.readInto(this->m_Version, "version");
		config.readInto(this->m_Description, "description");
		config.readInto(this->m_ProcessToMonitor, "process_to_monitor");
		config.readInto(this->m_StartScript, "start_script");
		config.readInto(this->m_StopScript, "stop_script");
		config.readInto(this->m_StopForcedScript, "stop_forced_script");
		config.readInto(this->m_EcmInfoParser, "ecm_info_parser");
		this->m_StopMaxDelayTime = config.read<int>("stop_max_delay_time");

		for (unsigned int i=0; i<this->m_Name.length(); i++)
			if (this->m_Name[i]  == '|')
				this->m_Name.replace(i, 1, " ");

		for (unsigned int i=0; i<this->m_Version.length(); i++)
			if (this->m_Version[i]  == '|')
				this->m_Version.replace(i, 1, " ");

		for (unsigned int i=0; i<this->m_Description.length(); i++)
			if (this->m_Description[i]  == '|')
				this->m_Description.replace(i, 1, " ");

		this->m_Initialized = true;
		this->m_Dir = path;
		this->m_Id = this->m_Dir.substr(this->m_Dir.find_last_of('/')+1);

		this->m_Messages->Send(EmuMessages::DEBUG, "---------------");
		this->m_Messages->Send(EmuMessages::DEBUG, "Directory: %s", this->m_Dir.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Id: %s", this->m_Id.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Name: %s", this->m_Name.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Version: %s", this->m_Version.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Description: %s", this->m_Description.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Process to monitor: %s", this->m_ProcessToMonitor.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Start script: %s", this->m_StartScript.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Stop script: %s", this->m_StopScript.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Stop forced script: %s", this->m_StopForcedScript.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "Stop max delay time: %d", this->m_StopMaxDelayTime);
		this->m_Messages->Send(EmuMessages::DEBUG, "Ecm info parser: %s", this->m_EcmInfoParser.c_str());
		this->m_Messages->Send(EmuMessages::DEBUG, "---------------");
	}
	catch (ConfigFile::file_not_found e)
	{
		this->m_Messages->Send(EmuMessages::ERROR, "Config file '%s' not found", e.filename.c_str());
	}
	catch (ConfigFile::key_not_found e)
	{
		this->m_Messages->Send(EmuMessages::ERROR, "Key '%s' not found in configuration", e.key.c_str());
	}
	delete confpath;
}

Emu::~Emu()
{

}

bool Emu::operator==(Emu *emu)
{
	const char *desc = emu->getDescription();
	if (strlen(desc) == strlen(this->m_Description.c_str()))
		if (memcmp(desc, this->m_Description.c_str(), strlen(desc)) == 0)
			return true;

	return false;
}

bool Emu::operator==(const char *emuid) { return this->m_Id == emuid; }

bool Emu::isInitialized() { return this->m_Initialized; }

const char* Emu::getDirectory()			{ return this->m_Dir.c_str(); }
const char* Emu::getId()				{ return this->m_Id.c_str(); }
const char* Emu::getName()				{ return this->m_Name.c_str(); }
const char* Emu::getVersion()			{ return this->m_Version.c_str(); }
const char* Emu::getDescription()		{ return this->m_Description.c_str(); }
const char* Emu::getProcessToMonitor()	{ return this->m_ProcessToMonitor.c_str(); }
const char* Emu::getStartScript()		{ return this->m_StartScript.c_str(); }
const char* Emu::getStopScript()		{ return this->m_StopScript.c_str(); }
const char* Emu::getStopForcedScript()	{ return this->m_StopForcedScript.c_str(); }
int Emu::getStopMaxDelayTime()			{ return this->m_StopMaxDelayTime; }
const char* Emu::getEcmInfoParser()		{ return this->m_EcmInfoParser.c_str(); }
