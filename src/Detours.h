#pragma once

#include <mq/Plugin.h>

#ifndef DETOUR_TRAMPOLINE_DEF_MEMBER
#define DETOUR_TRAMPOLINE_DEF_MEMBER(cls, ret, name, argtypes) DETOUR_TRAMPOLINE_DEF(ret, name, argtypes)
#endif
