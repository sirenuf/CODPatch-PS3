#pragma once 
#include <vsh/vshtask.hpp>

#include "Utils/Std.hpp"
#include "Memory/Memory.hpp"

/* Calculated maximum of storing all of the profile stats in memory at once is about 640 + 480 + ~500 (generous)
 * so roughly 1.6 Kb. I think the PS3's 256 Mb is okay with us allocating that instead of multiple filesystem calls.
 * 1 Mb = 1 * 1000 Kb.
 * Keep in mind of thread stack size (?) though. Maybe maximum of 
 */

/* TODO:
 * Make a while loop that polls until 0x01F9F11C (name) has been populated with current user's username.
 * This will ensure that player has connected to activision and stats won't get overwritten.
 * while polling make sure it also detects if PRX is shutting down.
 */
namespace CODMW2
{
	namespace UnlockAll
	{
		constexpr size_t unlockAllStart = 0x01FFA0C7;
		constexpr size_t unlockAllEnd = 0x01FFA2A7;
		constexpr size_t unlockAllbytes = unlockAllEnd - unlockAllStart; // 480 bytes

		uint8_t unlockAllStorage[unlockAllbytes];
	}
	
	static void Initialize()
	{
		vshtask::Notify("In multiplayer.");
		vshtask::Notify("Reading memory...");

		ReadCurrentStats();
	}

	/* Will read stats saved to filesystem. */
	static void ReadSavedStats()
	{

	}

	static void ReadCurrentStats()
	{
		// Storing unlock data.

		using namespace UnlockAll;
		
		// Sets the starting address of 0x01FFA0C7
		// and reads all the bytes from start to end 0x01FFA2A7
		// Puts all of read bytes in unlockAllStorage
		ReadProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)unlockAllStart, unlockAllStorage, sizeof(unlockAllStorage));
	}

	static void SaveStats()
	{

	}
}