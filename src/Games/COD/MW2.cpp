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

		std::set<std::string> buildGameIDsSet()
		{
			std::set<std::string> s;

			s.insert("BLUS30450");
			s.insert("BLUS30377");
			s.insert("BLUS30337");
			s.insert("BLUS30429");
			s.insert("BLES00683");
			s.insert("BLES00691");
			s.insert("BLES00690");
			s.insert("BLES00686");
			s.insert("BLES00685");
			s.insert("BLES00684");
			s.insert("BLES00687");
			s.insert("NPEB00731");
			s.insert("BLJM61006");
			s.insert("BLJM60191");

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

	void Run()
	{
		vshtask::Notify("In multiplayer.");

		while (!CODCommon::IsGameReady(MW2))
		{
			if (!g_FindActiveGame.IsGameRunning(MW2))
				return;
			libpsutil::sleep(200);
		}

		bool dataSaveFileExists = CODCommon::VerifyFilesystem(MW2);
		if (dataSaveFileExists)
			CODCommon::LoadSavedStats(MW2);

		while (g_FindActiveGame.IsGameRunning(MW2))
		{
			vshtask::Notify("Saving");

			CODCommon::SaveCurrentStats(MW2);
			libpsutil::sleep(10000);
		}

		vshtask::Notify("Exiting this.");
	}
}
