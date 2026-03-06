#include "MW3.hpp"

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
		u8 XUID[] = { 0xFF, 0x51, 0xB5, 0x7A, 0xF3, 0x8A, 0xB7, 0x45, 0x00 };
		u8 unknownPS3Id[] = { 0x78, 0x78, 0x78, 0x78, 0x70, 0x73, 0x33, 0x00 };

		u32 pid = g_FindActiveGame.GetRunningGameProcessId();

		char SpoofedUsername[] = { 0x53, 0x6B, 0x79, 0x5F, 0x5F, 0x56, 0x6F, 0x64, 0x6B, 0x61, 0x61, 0x00 };
		char psnUsername[32]{};

		ReadProcessMemory(pid, (void*)0x1BBBC2C, psnUsername, sizeof(psnUsername)); // Get and save current PSN username!


		WriteProcessMemory(pid, (void*)0x731460, SpoofedUsername, sizeof(SpoofedUsername));

		WriteProcessMemory(pid, (void*)0x731474, unknownPS3Id, sizeof(unknownPS3Id)); // I have no idea what this is. I think it is fingerprinting, so anti cheat mitigation?
		WriteProcessMemory(pid, (void*)0x8A8118, unknownPS3Id, sizeof(unknownPS3Id)); // It just zeroes out the values that was there. Spoofing works without doing this. Will keep it just for safety reasons.

		WriteProcessMemory(pid,  (void*)0x731E30,   XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x8A37C8,   XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x8A8220,   XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1864538,  XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1BBBC50,  XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1C12508,  XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1C488E8,  XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1D58C28,  XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x8A37F0,   XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1C15968,  XUID,  sizeof(XUID));
		WriteProcessMemory(pid,  (void*)0x1CF1C30,  XUID,  sizeof(XUID));

		u8 refreshProfile = 0x01;
		WriteProcessMemory(pid, (void*)0x1BBBC29, &refreshProfile, sizeof(refreshProfile)); // This triggers IW engine to "refresh" profile!

		WaitTillProfileRefreshDone(pid, 0x1BBBC2C, SpoofedUsername);

		WriteProcessMemory(pid, (void*)0x1BBBC2C, psnUsername, sizeof(psnUsername));
	}

	void Run()
	{
		while (!CODCommon::IsGameReady(MW3))
		{
			if (!g_FindActiveGame.IsGameRunning(MW3))
				return;
			libpsutil::sleep(200);
		}

		libpsutil::sleep(3000);
		SpoofXUID();
		
		do
			libpsutil::sleep(1000);
		while (g_FindActiveGame.IsGameRunning(MW3));
	}
}