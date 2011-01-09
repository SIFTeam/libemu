#include <iostream>
#include <string.h>

#include "CardReader.h"
#include "ConfigFile.h"

CardReader::CardReader(const char *path, EmuMessages *messages)
{
	this->m_Messages = messages;
	this->m_Initialized = false;

	char *confpath = new char[strlen(path) + strlen(CR_CONFIG) + 2];
	sprintf(confpath, "%s/%s", path, CR_CONFIG);
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
		this->m_Messages->Send(EmuMessages::DEBUG, "---------------");
	}
	catch (ConfigFile::file_not_found e)
	{
		this->m_Messages->Send(EmuMessages::ERROR, "Config file '%s' not found", e.filename.c_str());
		//this->m_Initialized = false;
	}
	catch (ConfigFile::key_not_found e)
	{
		this->m_Messages->Send(EmuMessages::ERROR, "Key '%s' not found in configuration", e.key.c_str());
		//this->m_Initialized = false;
	}
	delete confpath;
}

CardReader::~CardReader()
{

}

bool CardReader::operator==(CardReader *cr)
{
	const char *desc = cr->getDescription();
	if (strlen(desc) == strlen(this->m_Description.c_str()))
		if (memcmp(desc, this->m_Description.c_str(), strlen(desc)) == 0)
			return true;

	return false;
}

bool CardReader::operator==(const char *crid) { return this->m_Id == crid; }

bool CardReader::isInitialized() { return this->m_Initialized; }

const char* CardReader::getDirectory()			{ return this->m_Dir.c_str(); }
const char* CardReader::getId()					{ return this->m_Id.c_str(); }
const char* CardReader::getName()				{ return this->m_Name.c_str(); }
const char* CardReader::getVersion()			{ return this->m_Version.c_str(); }
const char* CardReader::getDescription()		{ return this->m_Description.c_str(); }
const char* CardReader::getProcessToMonitor()	{ return this->m_ProcessToMonitor.c_str(); }
const char* CardReader::getStartScript()		{ return this->m_StartScript.c_str(); }
const char* CardReader::getStopScript()			{ return this->m_StopScript.c_str(); }
const char* CardReader::getStopForcedScript()	{ return this->m_StopForcedScript.c_str(); }
int CardReader::getStopMaxDelayTime()			{ return this->m_StopMaxDelayTime; }
