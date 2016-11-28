#pragma once

#include <iplayerinfo.h>

// class IHandleEntity;
// class CBaseEntity;

// typedef int	string_t;
// #define STRING(offset) ((offset) ? reinterpret_cast<const char*>(offset) 

// struct color32
// {
// 	unsigned char r, g, b, a;
// };

class variant_t
{
	union
	{
		bool bVal;
		string_t iszVal;
		int iVal;
		float flVal;
		float vecVal[3];
		color32 rgbaVal;
	};

public:
	const char* String() const
	{
		return STRING(iszVal);
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
