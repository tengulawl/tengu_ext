#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include <smsdk_ext.h>
#include <CDetour/detours.h>

class CTenguExt : public SDKExtension
{
public:
	virtual bool SDK_OnLoad(char* error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();

#if defined SMEXT_CONF_METAMOD
	virtual bool SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	//virtual bool SDK_OnMetamodUnload(char* error, size_t maxlength);
#endif
};

#define DETOUR_DECL_MEMBER5(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name); \
        static ret (name##Class::* name##_Actual)(p1type, p2type, p3type, p4type, p5type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type, p4type, p5type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name)

#define DETOUR_CREATE_MEMBER_EX(detour, callback, signame) \
	detour = DETOUR_CREATE_MEMBER(callback, signame); \
	if (detour) { \
		detour->EnableDetour(); \
	} else { \
		smutils->LogError(myself, "Failed to hook %s", signame); \
	}

#define DETOUR_DESTROY_EX(detour) \
	if (detour) { \
		detour->Destroy(); \
		detour = nullptr; \
	}
	
#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
