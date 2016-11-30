#include "extension.h"
#include "hl2sdk_css.h"

CTenguExt g_Tengu;
SMEXT_LINK(&g_Tengu);

IGameConfig* g_game_config = nullptr;
IForward* g_should_hit_entity_forward = nullptr;
IForward* g_flashbang_detonate_forward = nullptr;
IForward* g_point_server_command_forward = nullptr;
IForward* g_can_join_team_forward = nullptr;
IForward* g_get_max_speed_forward = nullptr;
CDetour* g_should_hit_entity = nullptr;
CDetour* g_flashbang_detonate = nullptr;
CDetour* g_point_server_command = nullptr;
CDetour* g_join_team_command = nullptr;
CDetour* g_bot_add_command = nullptr;
CDetour* g_team_full_check = nullptr;
CDetour* g_get_player_max_speed = nullptr;
const int g_pass_entity_offset = 4;
const int g_collision_group_offset = 8;
cell_t g_join_team_player = 0;
bool g_in_bot_add_command = false;

DETOUR_DECL_MEMBER2(ShouldHitEntity, bool, IHandleEntity*, handle_entity, int, contents_mask)
{
	IHandleEntity* pass_entity = *(IHandleEntity**)((intptr_t)this + g_pass_entity_offset);
	int collision_group = *(int*)((intptr_t)this + g_collision_group_offset);

	if (!handle_entity || !pass_entity || handle_entity == pass_entity) {
		return DETOUR_MEMBER_CALL(ShouldHitEntity)(handle_entity, contents_mask);
	}

	if (g_should_hit_entity_forward->GetFunctionCount()) {
		cell_t touch_ent = gamehelpers->EntityToBCompatRef((CBaseEntity*)handle_entity);
		cell_t pass_ent = gamehelpers->EntityToBCompatRef((CBaseEntity*)pass_entity);
		cell_t result = false;
		cell_t pl_res = Pl_Continue;

		g_should_hit_entity_forward->PushCell(touch_ent);
		g_should_hit_entity_forward->PushCell(pass_ent);
		g_should_hit_entity_forward->PushCell(collision_group);
		g_should_hit_entity_forward->PushCell(contents_mask);
		g_should_hit_entity_forward->PushCellByRef(&result);
		g_should_hit_entity_forward->Execute(&pl_res);

		if (pl_res != Pl_Continue) {
			return res != 0;
		}
	}

	return DETOUR_MEMBER_CALL(ShouldHitEntity)(pHandleEntity, contentsMask);
}

DETOUR_DECL_MEMBER0(FlashbangDetonate, void)
{
	if (g_flashbang_detonate_forward->GetFunctionCount()) {
		cell_t entity = gamehelpers->EntityToBCompatRef((CBaseEntity*)this);
		cell_t pl_res = Pl_Continue;

		g_flashbang_detonate_forward->PushCell(entity);
		g_flashbang_detonate_forward->Execute(&pl_res);

		if (pl_res != Pl_Continue) {
			return;
		}
	}

	DETOUR_MEMBER_CALL(FlashbangDetonate)();
}

DETOUR_DECL_MEMBER1(PointServerCommand, void, inputdata_t&, inputdata)
{
	if (!inputdata.value.String()[0]) {
		return;
	}

	if (g_point_server_command_forward->GetFunctionCount()) {
		cell_t pl_res = Pl_Continue;

		g_point_server_command_forward->PushString(inputdata.value.String());
		g_point_server_command_forward->Execute(&pl_res);

		if (pl_res != Pl_Continue) {
			return;
		}
	}

	DETOUR_MEMBER_CALL(PointServerCommand)(inputdata);
}

DETOUR_DECL_MEMBER1(JoinTeamCommand, bool, int, team_id)
{
	g_join_team_player = gamehelpers->EntityToBCompatRef((CBaseEntity*)this);

	bool result = DETOUR_MEMBER_CALL(JoinTeamCommand)(team_id);

	g_join_team_player = 0;

	return result;
}

DETOUR_DECL_MEMBER5(BotAddCommand, bool, int, team_id, bool, is_from_console, const char*, profile_name, CSWeaponType, weapon_type, BotDifficultyType, difficulty)
{
	g_in_bot_add_command = true;

	bool result = DETOUR_MEMBER_CALL(BotAddCommand)(team_id, is_from_console, profile_name, weapon_type, difficulty);

	g_in_bot_add_command = false;

	return result;
}

DETOUR_DECL_MEMBER1(TeamFullCheck, bool, int, team_id)
{
	if (g_join_team_player || g_in_bot_add_command) {
		if (g_can_join_team_forward->GetFunctionCount()) {
			cell_t is_bot = true;
			cell_t result = false;
			cell_t pl_res = Pl_Continue;

			if (g_join_team_player && !g_in_bot_add_command) {
				IGamePlayer* player = playerhelpers->GetGamePlayer(g_join_team_player);

				if (player->IsSourceTV() || player->IsReplay()) {
					return DETOUR_MEMBER_CALL(TeamFullCheck)(team_id);
				}

				is_bot = player->IsFakeClient();
			}

			g_can_join_team_forward->PushCell(is_bot);
			g_can_join_team_forward->PushCell(team_id);
			g_can_join_team_forward->PushCellByRef(&result);
			g_can_join_team_forward->Execute(&pl_res);

			if (pl_res != Pl_Continue) {
				return !result;
			}
		}
	}

	return DETOUR_MEMBER_CALL(TeamFullCheck)(team_id);
}

DETOUR_DECL_MEMBER0(GetPlayerMaxSpeed, float)
{
	float result = DETOUR_MEMBER_CALL(GetPlayerMaxSpeed)();

	if (g_get_max_speed_forward->GetFunctionCount()) {
		cell_t client = gamehelpers->EntityToBCompatRef((CBaseEntity*)this);
		cell_t pl_res = Pl_Continue;
		float max_speed = result;
		
		g_get_max_speed_forward->PushCell(client);
		g_get_max_speed_forward->PushFloatByRef(&max_speed);
		g_get_max_speed_forward->Execute(&pl_res);

		if (pl_res != Pl_Continue) {
			return max_speed;
		}
	}

	return result;
}

bool CTenguExt::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
	char config_error[256];

	if (!gameconfs->LoadGameConfigFile("tengu.games", &g_game_config, config_error, sizeof(config_error))) {
		snprintf(error, maxlength, "Could not read tengu.games: %s", config_error);
		return false;
	}

	sharesys->RegisterLibrary(myself, "tengu");

	g_should_hit_entity_forward = forwards->CreateForward("OnShouldHitEntity", ET_Hook, 5, nullptr, Param_Cell, Param_Cell, Param_Cell, Param_Cell, Param_CellByRef);
	g_flashbang_detonate_forward = forwards->CreateForward("OnFlashbangDetonate", ET_Hook, 1, nullptr, Param_Cell);
	g_point_server_command_forward = forwards->CreateForward("OnPointServerCommand", ET_Hook, 1, nullptr, Param_String);
	g_can_join_team_forward = forwards->CreateForward("OnCanJoinTeam", ET_Hook, 3, nullptr, Param_Cell, Param_Cell, Param_CellByRef);
	g_get_max_speed_forward = forwards->CreateForward("OnGetPlayerMaxSpeed", ET_Hook, 2, nullptr, Param_Cell, Param_FloatByRef);

	CDetourManager::Init(smutils->GetScriptingEngine(), g_game_config);

	DETOUR_CREATE_MEMBER_EX(g_should_hit_entity, ShouldHitEntity, "CTraceFilterSimple::ShouldHitEntity");
	DETOUR_CREATE_MEMBER_EX(g_flashbang_detonate, FlashbangDetonate, "CFlashbangProjectile::Detonate");
	DETOUR_CREATE_MEMBER_EX(g_point_server_command, PointServerCommand, "CPointServerCommand::InputCommand");
	DETOUR_CREATE_MEMBER_EX(g_join_team_command, JoinTeamCommand, "CCSPlayer::HandleCommand_JoinTeam");
	DETOUR_CREATE_MEMBER_EX(g_bot_add_command, BotAddCommand, "CCSBotManager::BotAddCommand");
	DETOUR_CREATE_MEMBER_EX(g_team_full_check, TeamFullCheck, "CCSGameRules::TeamFull");
	DETOUR_CREATE_MEMBER_EX(g_get_player_max_speed, GetPlayerMaxSpeed, "CCSPlayer::GetPlayerMaxSpeed");

	return true;
}

void CTenguExt::SDK_OnUnload()
{
	DETOUR_DESTROY_EX(g_should_hit_entity);
	DETOUR_DESTROY_EX(g_flashbang_detonate);
	DETOUR_DESTROY_EX(g_point_server_command);
	DETOUR_DESTROY_EX(g_join_team_command);
	DETOUR_DESTROY_EX(g_bot_add_command);
	DETOUR_DESTROY_EX(g_team_full_check);
	DETOUR_DESTROY_EX(g_get_player_max_speed);

	forwards->ReleaseForward(g_should_hit_entity_forward);
	forwards->ReleaseForward(g_flashbang_detonate_forward);
	forwards->ReleaseForward(g_point_server_command_forward);
	forwards->ReleaseForward(g_can_join_team_forward);
	forwards->ReleaseForward(g_get_max_speed_forward);

	gameconfs->CloseGameConfigFile(g_game_config);
}
