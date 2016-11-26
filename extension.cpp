#include "extension.h"
#include "hl2sdk_css.h"

CTenguExt g_Tengu;
SMEXT_LINK(&g_Tengu);

IGameConfig* g_gameConfig = nullptr;
IForward* g_shouldHitEntityForward = nullptr;
IForward* g_flashbangDetonateForward = nullptr;
IForward* g_pointServerCommandForward = nullptr;
IForward* g_canBotJoinTeamForward = nullptr;
IForward* g_canJoinTeamForward = nullptr;
CDetour* g_shouldHitEntity = nullptr;
CDetour* g_flashbangDetonate = nullptr;
CDetour* g_pointServerCommand = nullptr;
CDetour* g_joinTeamCommand = nullptr;
CDetour* g_botAddCommand = nullptr;
CDetour* g_teamFullCheck = nullptr;
CDetour* g_teamStackedCheck = nullptr;
const int g_passEntOffset = 4;
const int g_collisionOffs = 8;
cell_t g_joinTeamPlayer = 0;
bool g_inBotAddCommand = false;

DETOUR_DECL_MEMBER2(MyShouldHitEntity, bool, IHandleEntity*, pHandleEntity, int, contentsMask)
{
	IHandleEntity* m_pPassEnt = *(IHandleEntity**)((intptr_t)this + g_passEntOffset);
	int collisionGroup = *(int*)((intptr_t)this + g_collisionOffs);

	if (!pHandleEntity || !m_pPassEnt || pHandleEntity == m_pPassEnt) {
		return DETOUR_MEMBER_CALL(MyShouldHitEntity)(pHandleEntity, contentsMask);
	}

	if (g_shouldHitEntityForward->GetFunctionCount()) {
		cell_t touchEnt = gamehelpers->EntityToBCompatRef((CBaseEntity*)pHandleEntity);
		cell_t passEnt = gamehelpers->EntityToBCompatRef((CBaseEntity*)m_pPassEnt);
		cell_t ret = 0, result = Pl_Continue;

		g_shouldHitEntityForward->PushCell(touchEnt);
		g_shouldHitEntityForward->PushCell(passEnt);
		g_shouldHitEntityForward->PushCell(collisionGroup);
		g_shouldHitEntityForward->PushCell(contentsMask);
		g_shouldHitEntityForward->PushCellByRef(&ret);
		g_shouldHitEntityForward->Execute(&result);

		if (result != Pl_Continue) {
			return (ret != 0);
		}
	}

	return DETOUR_MEMBER_CALL(MyShouldHitEntity)(pHandleEntity, contentsMask);
}

DETOUR_DECL_MEMBER0(MyFlashbangDetonate, void)
{
	if (g_flashbangDetonateForward->GetFunctionCount()) {
		cell_t entindex = gamehelpers->EntityToBCompatRef((CBaseEntity*)this);
		cell_t result = Pl_Continue;

		g_flashbangDetonateForward->PushCell(entindex);
		g_flashbangDetonateForward->Execute(&result);

		if (result != Pl_Continue) {
			return;
		}
	}

	DETOUR_MEMBER_CALL(MyFlashbangDetonate)();
}

DETOUR_DECL_MEMBER1(MyPointServerCommand, void, inputdata_t&, inputdata)
{
	if (!inputdata.value.String()[0]) {
		return;
	}

	if (g_pointServerCommandForward->GetFunctionCount()) {
		cell_t result = Pl_Continue;

		g_pointServerCommandForward->PushString(inputdata.value.String());
		g_pointServerCommandForward->Execute(&result);

		if (result != Pl_Continue) {
			return;
		}
	}

	DETOUR_MEMBER_CALL(MyPointServerCommand)(inputdata);
}

DETOUR_DECL_MEMBER1(MyJoinTeamCommand, bool, int, team)
{
	g_joinTeamPlayer = gamehelpers->EntityToBCompatRef((CBaseEntity*)this);

	bool ret = DETOUR_MEMBER_CALL(MyJoinTeamCommand)(team);

	g_joinTeamPlayer = 0;

	return ret;
}

DETOUR_DECL_MEMBER5(MyBotAddCommand, bool, int, team, bool, isFromConsole, const char*, profileName, CSWeaponType, weaponType, BotDifficultyType, difficulty)
{
	g_inBotAddCommand = true;

	bool ret = DETOUR_MEMBER_CALL(MyBotAddCommand)(team, isFromConsole, profileName, weaponType, difficulty);

	g_inBotAddCommand = false;

	return ret;
}

DETOUR_DECL_MEMBER1(MyTeamFullCheck, bool, int, teamId)
{
	if (g_joinTeamPlayer) {
		if (g_canJoinTeamForward->GetFunctionCount()) {
			cell_t isBot = 1, ret = 0, result = Pl_Continue;

			g_canJoinTeamForward->PushCell(g_joinTeamPlayer);
			g_canJoinTeamForward->PushCell(teamId);
			g_canJoinTeamForward->PushCellByRef(&ret);
			g_canJoinTeamForward->Execute(&result);

			if (result != Pl_Continue) {
				return !ret;
			}
		}
	} else if (g_inBotAddCommand) {
		if (g_canBotJoinTeamForward->GetFunctionCount()) {
			cell_t ret = 0, result = Pl_Continue;

			g_canBotJoinTeamForward->PushCell(teamId);
			g_canBotJoinTeamForward->PushCellByRef(&ret);
			g_canBotJoinTeamForward->Execute(&result);

			if (result != Pl_Continue) {
				return !ret;
			}
		}
	}

	return DETOUR_MEMBER_CALL(MyTeamFullCheck)(teamId);
}

bool CTenguExt::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
	char configError[256];

	if (!gameconfs->LoadGameConfigFile("tengu.games", &g_gameConfig, configError, sizeof(configError))) {
		snprintf(error, maxlength, "Could not read tengu.games: %s", configError);
		return false;
	}

	sharesys->RegisterLibrary(myself, "tengu");

	g_shouldHitEntityForward = forwards->CreateForward("OnShouldHitEntity", ET_Hook, 5, nullptr, Param_Cell, Param_Cell, Param_Cell, Param_Cell, Param_CellByRef);
	g_flashbangDetonateForward = forwards->CreateForward("OnFlashbangDetonate", ET_Hook, 1, nullptr, Param_Cell);
	g_pointServerCommandForward = forwards->CreateForward("OnPointServerCommand", ET_Hook, 1, nullptr, Param_String);
	g_canBotJoinTeamForward = forwards->CreateForward("OnCanBotJoinTeam", ET_Hook, 2, nullptr, Param_Cell, Param_CellByRef);
	g_canJoinTeamForward = forwards->CreateForward("OnCanJoinTeam", ET_Hook, 3, nullptr, Param_Cell, Param_Cell, Param_CellByRef);

	CDetourManager::Init(smutils->GetScriptingEngine(), g_gameConfig);

	DETOUR_CREATE_MEMBER_EX(g_shouldHitEntity, MyShouldHitEntity, "CTraceFilterSimple::ShouldHitEntity");
	DETOUR_CREATE_MEMBER_EX(g_flashbangDetonate, MyFlashbangDetonate, "CFlashbangProjectile::Detonate");
	DETOUR_CREATE_MEMBER_EX(g_pointServerCommand, MyPointServerCommand, "CPointServerCommand::InputCommand");
	DETOUR_CREATE_MEMBER_EX(g_joinTeamCommand, MyJoinTeamCommand, "CCSPlayer::HandleCommand_JoinTeam");
	DETOUR_CREATE_MEMBER_EX(g_botAddCommand, MyBotAddCommand, "CCSBotManager::BotAddCommand");
	DETOUR_CREATE_MEMBER_EX(g_teamFullCheck, MyTeamFullCheck, "CCSGameRules::TeamFull");

	return true;
}

void CTenguExt::SDK_OnUnload()
{
	DETOUR_DESTROY_EX(g_shouldHitEntity);
	DETOUR_DESTROY_EX(g_flashbangDetonate);
	DETOUR_DESTROY_EX(g_pointServerCommand);
	DETOUR_DESTROY_EX(g_joinTeamCommand);
	DETOUR_DESTROY_EX(g_botAddCommand);
	DETOUR_DESTROY_EX(g_teamFullCheck);

	forwards->ReleaseForward(g_shouldHitEntityForward);
	forwards->ReleaseForward(g_flashbangDetonateForward);
	forwards->ReleaseForward(g_pointServerCommandForward);
	forwards->ReleaseForward(g_canBotJoinTeamForward);
	forwards->ReleaseForward(g_canJoinTeamForward);

	gameconfs->CloseGameConfigFile(g_gameConfig);
}
