#include "pch-il2cpp.h"
#include "Commands.h"
#include "Server.h"

std::map<std::string, CommandInfo*> Commands::m_CommandInfos;
float Commands::mapModeTime = 0;
float Commands::allowStartTime;

void Commands::IncrementMapModeTime(float dt) {
	Commands::mapModeTime += dt;
}

void Commands::IncrementAllowStartTime(float dt) {
	if (Server::InLobby) {
		Commands::allowStartTime += dt;
	}
	else {
		Commands::allowStartTime = 0;
	}
}

bool Commands::GetCommandInfo(std::string cmd, CommandInfo* &info) {

	std::map<std::string, CommandInfo*>::iterator it;
	for (it = m_CommandInfos.begin(); it != m_CommandInfos.end(); it++)
	{
		if (cmd.compare(it->first) == 0) {
			info = it->second;
			return true;
		}
	}

	return false;
}

CommandInfo* Commands::RegisterCommand(std::string cmd, std::string perms, bool hide) {
	CommandInfo* info = new CommandInfo();

	m_CommandInfos.insert(std::pair<std::string, CommandInfo*>(cmd, info));

	info->m_HideCommand = hide;

	std::vector<std::string> elements;
	split(perms, ';', std::back_inserter(elements));

	for (size_t i = 0; i < elements.size(); i++)
	{
		info->m_Permissions.push_back(elements[i]);
	}


	return info;
}