#if defined _tengu_included
 #endinput
#endif
#define _tengu_included

forward Action OnShouldHitEntity(int touchEnt, int passEnt, int collisionGroup, int contentsMask, bool& result);
forward Action OnFlashbangDetonate(int entity);
forward Action OnPointServerCommand(const char[] command);
forward Action OnCanJoinTeam(bool isBot, int team, bool& result);

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
