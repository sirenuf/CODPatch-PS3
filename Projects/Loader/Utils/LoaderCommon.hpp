#pragma once
#include "Utils/Types.hpp"

#include "Utils/Globals.hpp"

namespace LoaderCommon
{
	Globals::Game GetGameType();
	void HandleGames(Globals::Game gameType, bool bPatchGames);
	u32 FindFunctionAddress(u32 address, const i16* signature, u32 sigLen, u32 scanSize);
}