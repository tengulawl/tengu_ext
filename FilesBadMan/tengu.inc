#if defined _tengu_included
 #endinput
#endif
#define _tengu_included

forward Action OnShouldHitEntity(int touch_entity, int pass_entity, int collision_group, int contents_mask, bool& result);
forward Action OnFlashbangDetonate(int entity);
forward Action OnPointServerCommand(const char[] command);
forward Action OnCanJoinTeam(bool is_bot, int team_id, bool& result);
forward Action OnGetPlayerMaxSpeed(int client, float& max_speed);

public Extension __ext_tengu = {
	name = "tengu",
	file = "tengu.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};
