#include "MW3.hpp"
#include <vsh/vshtask.hpp>

namespace MW3
{
	// Privates
	namespace {
		std::set<std::string> buildGameIDsSet()
		{
			std::set<std::string> s;

			s.insert("BLES01428");
			s.insert("BLES01430");
			s.insert("BLES01431");
			s.insert("BLES01432");
			s.insert("BLES01433");
			s.insert("BLES01434");
			s.insert("BLUS30838");
			s.insert("NPUB30787");
			s.insert("BLES01429");
			s.insert("BLJM60404");
			s.insert("NPEB00967");

			return s;
		};
	}

	const std::set<std::string>& GetGameIDs()
	{
		static std::set<std::string> s = buildGameIDsSet();
		return s;
	}
	
	/**
	 * Sleeps the thread till the profile refresh is done
	 * @return
	 */
	void WaitTillProfileRefreshDone(u32& pid, u32 address, char* spoofedUsername)
	{
		char buffer[32]{};

		do {
			ReadProcessMemory(pid, (void*)address, buffer, sizeof(buffer));
			libpsutil::sleep(500);
		} while (!strcmp(buffer, spoofedUsername));
	}

	void SpoofXUID()
	{
		u8 XUID[] = { 0xC0, 0xCC, 0x50, 0xCB, 0x3B, 0xF9, 0xD6, 0x11 };
		u8 unknownPS3Id[] = { 0x78, 0x78, 0x78, 0x78, 0x70, 0x73, 0x33, 0x00 };

		u32 pid = g_FindActiveGame.GetRunningGameProcessId();

		char SpoofedUsername[] = { 0x53, 0x6B, 0x79, 0x5F, 0x5F, 0x56, 0x6F, 0x64, 0x6B, 0x61, 0x61, 0x00 };
		char psnUsername[32]{};

		WriteProcessMemory(pid, (void*)0x731e30, XUID, sizeof(XUID));
		
		for (int i = 0; i < 18; i++)
			WriteProcessMemory(pid, (void*)(0x8a8220 + i * 0x178), XUID, sizeof(XUID));

		vshtask::Notify("spoofing shit");

		WriteProcessMemory(pid, (void*)0x1BBBC50,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1C11D48,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1C12300,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1C12508,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1CF1C30,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1C488E8,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1D4DFA0,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1D58C28,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x1D8ED58,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x246B3730,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x346B6910,	XUID,	sizeof(XUID));
		WriteProcessMemory(pid, (void*)0x28D21428,	XUID,	sizeof(XUID));
		
		// Refresh profile
		u8 refreshProfileByte = 0x01;
		WriteProcessMemory(pid, (void*)0x1BBBC2B, &refreshProfileByte, sizeof(refreshProfileByte));
	}

	void sDwFetchPerformanceValuesComplete(...)
	{
		__asm("li %r3, 0x0;");
	}

	void DwFetchPerformanceValuesComplete(int task, int playerRanks, int* numPlayerRanks)
	{
		return;
	}

	void Run()
	{
		while (!CODCommon::IsGameReady(MW3))
		{
			if (!g_FindActiveGame.IsGameRunning(MW3))
				return;
			libpsutil::sleep(200);
		}

		vshtask::Notify("Waiting for hooking.");
		libpsutil::sleep(3000);
		HookFunctionStart(0x340C8C, (int)sDwFetchPerformanceValuesComplete, (int)DwFetchPerformanceValuesComplete);
		//SpoofXUID();
		
		do
			libpsutil::sleep(1000);
		while (g_FindActiveGame.IsGameRunning(MW3));
	}
}