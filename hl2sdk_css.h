#pragma once

#include <ihandleentity.h>

class variant_t
{
	const char* pszValue;

public:
	inline const char* String() const
	{
		return (pszValue) ? pszValue : "";
	}
};

struct inputdata_t
{
	CBaseEntity* pActivator;
	CBaseEntity* pCaller;
	variant_t value;
	int nOutputID;
};

typedef int CSWeaponType;
typedef int BotDifficultyType;
