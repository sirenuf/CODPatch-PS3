#include "MW2.hpp"
#include "Games/FindActiveGame.hpp"

#include <vector>

namespace MW2
{
	using CODCommon::MW2;
	using CODCommon::MemoryEntry;
	using CODCommon::MemoryMapping;

	// Anonymous namespace to keep setup function isolated
	namespace
	{
		// FYI: Have to do it this way because of map implementation on SDK
		std::vector<MemoryMapping> buildMemoryArray()
		{
			std::vector<MemoryMapping> a;

			a.push_back(MemoryMapping("Prestige",								MemoryEntry( 0x01FF9A9C, 1   )));
			a.push_back(MemoryMapping("Experience",								MemoryEntry( 0x01FF9A94, 4   )));
			a.push_back(MemoryMapping("Score",									MemoryEntry( 0x01FF9AA4, 4   )));

			a.push_back(MemoryMapping("Wins",									MemoryEntry( 0x01FF9ADC, 4   )));
			a.push_back(MemoryMapping("Losses",									MemoryEntry( 0x01FF9AE0, 4   )));
			a.push_back(MemoryMapping("Ties",									MemoryEntry( 0x01FF9AE4, 4   )));
			a.push_back(MemoryMapping("WinStreak",								MemoryEntry( 0x01FF9AE8, 4   )));

			a.push_back(MemoryMapping("Kills",									MemoryEntry( 0x01FF9AA8, 4   )));
			a.push_back(MemoryMapping("HeadShots",								MemoryEntry( 0x01FF9ABC, 4   )));
			a.push_back(MemoryMapping("Assists",								MemoryEntry( 0x01FF9AB8, 4   )));
			a.push_back(MemoryMapping("Streak",									MemoryEntry( 0x01FF9AAC, 4   )));

			a.push_back(MemoryMapping("Deaths",									MemoryEntry( 0x01FF9AB0, 4   )));
			a.push_back(MemoryMapping("TimePlayed",								MemoryEntry( 0x01FF9AC8, 12  )));

			a.push_back(MemoryMapping("Accolades",								MemoryEntry( 0x01FF9297, 440 )));
			a.push_back(MemoryMapping("Unlocks",								MemoryEntry( 0x01FFA0C7, 480 )));
			a.push_back(MemoryMapping("ClassData",								MemoryEntry( 0x01FF9E40, 640 )));

			a.push_back(MemoryMapping("ClanTag",								MemoryEntry( 0x00A053A4, 4   )));
			a.push_back(MemoryMapping("ClanTagMenu",							MemoryEntry( 0x00A11422, 4   )));

			a.push_back(MemoryMapping("Emblem",									MemoryEntry( 0x01FFAAC7, 2   )));
			a.push_back(MemoryMapping("Title",									MemoryEntry( 0x01FFAAC9, 2   )));

			a.push_back(MemoryMapping("KillStreakRewards_Equipped",				MemoryEntry( 0x01FFAACD, 6   )));
			a.push_back(MemoryMapping("KillStreakRewards_Unlocked",				MemoryEntry( 0x01FFACF3, 4   )));
			a.push_back(MemoryMapping("KillStreakRewards_PointsAvailable",		MemoryEntry( 0x01C1EEDB, 1   )));

			a.push_back(MemoryMapping("ChallengeProgressData",					MemoryEntry( 0x01FFA41B, 1552 )));

			return a;
		}

		// TODO:
		// GameIDs might be an unreliable way of finding out what game it is.
		// Consider changing to just string pattern recognition.
		std::set<std::string> buildGameIDsSet()
		{
			std::set<std::string> s;

			s.insert("BLES00690");
			s.insert("BLKS20159");
			s.insert("BLES00684");
			s.insert("BLUS30377");
			s.insert("NPEB00731");
			s.insert("NPEB00733");
			s.insert("BLES00691");
			s.insert("BLES00683");
			s.insert("BLUS30449");
			s.insert("NPEB00735");
			s.insert("BLUS30450");
			s.insert("BLES00686");
			s.insert("NPEB00732");
			s.insert("NPUB30585");
			s.insert("BLES00687");
			s.insert("BLJM61006");
			s.insert("BLUS30429");
			s.insert("BLES00685");
			s.insert("BCKS10186");
			s.insert("NPEB00734");
			s.insert("BLJM60191");
			s.insert("NPUB90477");
			s.insert("BLJM60269");

			return s;
		};
	}

	const std::vector<MemoryMapping>& GetMemory()
	{
		static std::vector<MemoryMapping> m = buildMemoryArray();
		return m;
	}

	const std::set<std::string>& GetGameIDs()
	{
		static std::set<std::string> s = buildGameIDsSet();
		return s;
	}

	bool AccountCanSave()
	{
		u32 level{};
		ReadProcessMemory(vsh::GetGameProcessId(), (void*)0x01FF9A94, &level, 3);
		
		return level > 0;
	}

	// This manually applies the clan tag in the menu to correspond with set clan tag.
	// This is entirely visual, and only here for consistency and perfectionism.
	void ApplyClanTagInMenu()
	{
		char clanTag[4]{};

		// Save current clan tag
		ReadProcessMemory(vsh::GetGameProcessId(), (void*)0x00A053A4, clanTag, sizeof(clanTag));

		// Write to clan tag menu
		WriteProcessMemory(vsh::GetGameProcessId(), (void*)0x00A11422, clanTag, sizeof(clanTag));
	}

	void Run()
	{
		while (!CODCommon::IsGameReady(MW2))
		{
			if (!g_FindActiveGame.IsGameRunning(MW2))
				return;
			libpsutil::sleep(200);
		}

		// In case a user that has already an account that is able to save data, stop execution so we don't overwrite his data.
		if (AccountCanSave())
		{
			vshtask::Notify("CODPatch: Warning! This account can save stats. Freezing execution.");
			while (true)
			{
				if (!g_FindActiveGame.IsGameRunning(MW2))
					return;

				libpsutil::sleep(10000);
			}
		}

		bool dataSaveFileExists = CODCommon::VerifyFilesystem(MW2);
		if (dataSaveFileExists)
			CODCommon::LoadSavedStats(MW2);

		libpsutil::sleep(3000);
		ApplyClanTagInMenu();

		while (g_FindActiveGame.IsGameRunning(MW2))
		{
			CODCommon::SaveCurrentStats(MW2);
			libpsutil::sleep(10000);
		}
	}
}
