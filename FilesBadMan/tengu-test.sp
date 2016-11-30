#include <sourcemod>
#include <sdktools>
#include <cstrike>
#include <tengu>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
	name        = "tengu-test",
	author      = "Tengu",
	description = "Tengu is love, Tengu is life",
	version     = "0.2",
	url         = "https://steamcommunity.com/id/tengulawl"
};

public Action OnShouldHitEntity(int touch_entity, int pass_entity, int collision_group, int contents_mask, bool& result)
{
	if ((0 < touch_entity <= MaxClients) && (0 < pass_entity <= MaxClients)) {
		if (IsFakeClient(touch_entity) || IsFakeClient(pass_entity)) {
			result = false;
			return Plugin_Handled;
		}
	}

	return Plugin_Continue;
}

public Action OnFlashbangDetonate(int entity)
{
	AcceptEntityInput(entity, "Kill");
	return Plugin_Handled;
}

public Action OnPointServerCommand(const char[] command)
{
	PrintToServer("Command prevented: %s", command);
	return Plugin_Handled;
}

public Action OnCanJoinTeam(bool is_bot, int team_id, bool& result)
{
	if (is_bot) {
		result = (team_id != CS_TEAM_CT);
	} else {
		result = (team_id != CS_TEAM_T);
	}

	return Plugin_Handled;
}

public Action OnGetPlayerMaxSpeed(int client, float& max_speed)
{
	if (!IsFakeClient(client)) {
		max_speed *= 2.0;
		return Plugin_Changed;
	}

	return Plugin_Continue;
}
