#include "LoaderCommon.hpp"

#include "Utils/Globals.hpp"
#include "Memory/Memory.hpp"

#include "GamePatches/MW2/MW2.hpp"
#include "GamePatches/MW3/MW3.hpp"
#include "GamePatches/BO1/BO1.hpp"
#include "GamePatches/BO2/BO2.hpp"

#include <libpsutil.h>

namespace LoaderCommon
{
	using namespace Globals;

	Game GetGameType()
	{
		u32 gameType{};
		libpsutil::filesystem::read_file(GetIpcCacheFile(), &gameType, sizeof(gameType));

		return (Game)gameType;
	}

	void HandleGames(Game gameType, bool bPatchGames)
	{
		switch (gameType)
		{
		case Game::MW2:
			printf("[CODPatch Loader] Game is: MW2 (IW4)\n");
			if (bPatchGames)
				MW2::Main();
			break;
		case Game::MW3:
			printf("[CODPatch Loader] Game is: MW3 (IW5)\n");
			if (bPatchGames)
				MW3::Main();
			break;
		case Game::BO1:
			printf("[CODPatch Loader] Game is: BO1 (T5)\n");
			if (bPatchGames)
				BO1::Main();
			break;
		case Game::BO2:
			printf("[CODPatch Loader] Game is: BO2 (T6)\n");
			if (bPatchGames)
				BO2::Main();
			break;
		default:
			printf("[CODPatch Loader] No game supported game found. gameType: %d. Quitting.\n", gameType);
			break;
		}
	}

	u32 FindFunctionAddress(u32 address, const i16* signature, u32 sigLen, u32 scanSize)
	{
		if (sigLen == 0 || sigLen > scanSize)
			return 0;

		u32 base = address - (scanSize / 2);
		u8* buffer = new u8[scanSize];

		if (ReadProcessMemory(sys_process_getpid(), (void*)base, buffer, scanSize) != 0)
		{
			delete[] buffer;
			return 0;
		}

		for (u32 i = 0; i + sigLen <= scanSize; ++i)
		{
			u32 j = 0;
			while (j < sigLen)
			{
				if (signature[j] >= 0 && buffer[i + j] != (u8)signature[j])
					break;
				++j;
			}

			if (j == sigLen)
			{
				delete[] buffer;
				return base + i;
			}
		}

		delete[] buffer;
		return 0;
	}
}