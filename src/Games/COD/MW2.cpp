#include "MW2.hpp"
#include "Games/FindActiveGame.hpp"

namespace MW2
{
	using CODCommon::MW2;
	using CODCommon::MemoryEntry;

	// Anonymous namespace to keep setup function isolated
	namespace
	{
		// FYI: Have to do it this way because of map implementation on SDK
		std::hash_map<std::string, MemoryEntry> buildMemoryMap()
		{
			std::hash_map<std::string, MemoryEntry> m;

			m.insert(std::make_pair("Prestige",		MemoryEntry{ 0x01FF9A9C, 1		}));
			m.insert(std::make_pair("Experience",	MemoryEntry{ 0x01FF9A94, 4		}));
			m.insert(std::make_pair("Score",		MemoryEntry{ 0x01FF9AA4, 4		}));

			m.insert(std::make_pair("Wins",			MemoryEntry{ 0x01FF9ADC, 4		}));
			m.insert(std::make_pair("Losses",		MemoryEntry{ 0x01FF9AE0, 4		}));
			m.insert(std::make_pair("Ties",			MemoryEntry{ 0x01FF9AE4, 4		}));
			m.insert(std::make_pair("WinStreak",	MemoryEntry{ 0x01FF9AE8, 4		}));

			m.insert(std::make_pair("Kills",		MemoryEntry{ 0x01FF9AA8, 4		}));
			m.insert(std::make_pair("HeadShots",	MemoryEntry{ 0x01FF9ABC, 4		}));
			m.insert(std::make_pair("Assists",		MemoryEntry{ 0x01FF9AB8, 4		}));
			m.insert(std::make_pair("Streak",		MemoryEntry{ 0x01FF9AAC, 4		}));

			m.insert(std::make_pair("Deaths",		MemoryEntry{ 0x01FF9AB0, 4		}));
			m.insert(std::make_pair("TimePlayed",	MemoryEntry{ 0x01FF9AC8, 12		}));

			m.insert(std::make_pair("Accolades",	MemoryEntry{ 0x01FF9297, 440	}));
			m.insert(std::make_pair("Unlocks",		MemoryEntry{ 0x01FFA0C7, 480	}));
			m.insert(std::make_pair("ClassData",	MemoryEntry{ 0x01FF9E40, 640	}));

			return m;
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
		}
	}

	const std::hash_map<std::string, MemoryEntry>& GetMemory()
	{
		static std::hash_map<std::string, MemoryEntry> m = buildMemoryMap();
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
			// Check if this works by going in n out of game later
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