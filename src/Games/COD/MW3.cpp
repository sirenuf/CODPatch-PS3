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

	void Run()
	{
		// Zeroes out the function that kicks you out if your XUID is "invalid".
		// 38 60 00 00   -> li r3, 0
		// 4E 80 00 20   -> blr
		u8 functionOverwrite[] = { 0x38, 0x60, 0x00, 0x00, 0x4E, 0x80, 0x00, 0x20 };
		WriteProcessMemory(vsh::GetGameProcessId(), (void*)0x340C8C, functionOverwrite, sizeof(functionOverwrite));

		do
			libpsutil::sleep(1000);
		while (g_FindActiveGame.IsGameRunning(MW3));
	}
}