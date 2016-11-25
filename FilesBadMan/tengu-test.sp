#include <sourcemod>
#include <tengu>
#include <cstrike>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
	name        = "tengu-test",
	author      = "Tengu",
	description = "Tengu is love, Tengu is life",
	version     = "1.0",
	url         = "https://steamcommunity.com/id/tengulawl"
};

public Action OnShouldHitEntity(int touchEnt, int passEnt, int collisionGroup, int contentsMask, bool& result)
{
	if ((0 < touchEnt <= MaxClients) && (0 < passEnt <= MaxClients)) {
		if (IsFakeClient(touchEnt) || IsFakeClient(passEnt)) {
			result = false;
			return Plugin_Handled;
		}
	}

	return Plugin_Continue;
}

public Action OnFlashbangDetonate(int entity)
{
	return Plugin_Handled;
}

public Action OnPointServerCommand(const char[] command)
{
	PrintToServer("Command prevented: %s", command);
	return Plugin_Handled;
}

public bool OnCanJoinTeam(bool isBot, int team, bool originalResult)
{
	if (isBot) {
		if (team == CS_TEAM_T) {
			return true;
		}

		if (team == CS_TEAM_CT) {
			return false;
		}
	} else {
		if (team == CS_TEAM_T) {
			return false;
		}

		if (team == CS_TEAM_CT) {
			return true;
		}
	}

	return originalResult;
}
