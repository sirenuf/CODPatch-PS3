#include "MW2.hpp"

#include "Utils/LoaderCommon.hpp"
#include "Utils/Types.hpp"

#include <libpsutil.h>

/* Assembled from Assembly/Cave.s. Plain labels in .text rather than functions, so these
 * addresses are the instructions themselves and not function descriptors. */
extern "C" u8 mw2_cave_start[];
extern "C" u8 mw2_cave_end[];

namespace MW2
{
	namespace
	{
		/* Multiplayer executable: the successful Demonware auth response. The
		 * instruction living here is lwz r0, 0x98(r1), which the cave replays. */
		const u32 HookAddress = 0x004033F0;

		const i16 HookAddressSignature[] = {
			0x80, 0x01, 0x00, 0x98, 0x90, 0x19, 0x00, 0x24, 0x81, 0x21, 0x00, 0x70, 0x91, 0x39, 0x00, 0x28,
			0x4B, 0xFF, 0xFE, 0xAC, 0x60, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x04, 0x7F, 0xE4, 0xFB, 0x78,
			0x39, 0x01, 0x00, 0x73, 0x7C, 0x09, 0x03, 0xA6, 0x39, 0x40, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
			0x7D, 0x2A, 0x22, 0x14, 0x79, 0x0B, 0x00, 0x20, 0x79, 0x29, 0x00, 0x20, 0x39, 0x4A, 0x00, 0x01
		};
		const u32 HookAddressLength = 64; // 0x40 bytes

		/* Free space in the same executable that the cave is copied into. */
		const u32 CaveAddress = 0x006EC100;

		u32 Branch(u32 from, u32 to)
		{
			return 0x48000000u | ((to - from) & 0x03FFFFFC);
		}
	}

	void Main()
	{
		// Guard
		static bool patched = false;
		if (patched)
			return;

		/* This address should be static.Like MW3 we just use this as a guard
		 * to make sure we're not injecting in like default.self without relying
		 * on binary names which are inconsistent. Some people use default_mp.self
		 * as EBOOT.BIN because of baked in dynamic linking for their mod menus etc. 
		 * https://github.com/NotNite/SPRXPatcher there is no reason for it to be
		 * strictly called EBOOT.BIN though lol. At least I think? Honestly I don't care.
		 * World's a fuck or something. */ 
		u32 hookSignatureFound = LoaderCommon::FindFunctionAddress(HookAddress, HookAddressSignature, HookAddressLength, 512);

		if (hookSignatureFound == 0)
		{
			printf("[CODPatch Loader] MW2: Hook signature could not be located, which means we're quitting for this executable.\n");
			return;
		}

		const u32 bodySize = (u32)mw2_cave_end - (u32)mw2_cave_start;
		const u32 tailAddress = CaveAddress + bodySize;

		/* The cave goes down first - the game must never be able to branch into a
		 * half written one. */
		if (!libpsutil::memory::set(CaveAddress, mw2_cave_start, bodySize))
		{
			printf("[CODPatch Loader] MW2: could not write the cave at 0x%08X\n", CaveAddress);
			return;
		}

		/* Its last instruction returns to the game. It only encodes correctly at
		 * this exact address, which is why it is built here and not in the .s. */
		if (!libpsutil::memory::set<u32>(tailAddress, Branch(tailAddress, HookAddress + 4)))
		{
			printf("[CODPatch Loader] MW2: could not write the cave exit at 0x%08X\n", tailAddress);
			return;
		}

		/* Only now redirect the game, replacing the one instruction the cave replays. */
		if (!libpsutil::memory::set<u32>(HookAddress, Branch(HookAddress, CaveAddress)))
		{
			printf("[CODPatch Loader] MW2: could not patch 0x%08X\n", HookAddress);
			return;
		}

		patched = true;

		printf("[CODPatch Loader] MW2: auth user id cached (cave at 0x%08X, %u bytes)\n",
			CaveAddress, bodySize + 4);
	}
}
