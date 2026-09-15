#include "MW2.hpp"

#include <set>

#include "Utils/SystemCalls.hpp"
#include "Games/FindActiveGame.hpp"
#include "Games/Common.hpp"

#include <libpsutil.h>

namespace MW2
{
	namespace {
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

	const std::set<std::string>& GetGameIDs()
	{
		static std::set<std::string> s = buildGameIDsSet();
		return s;
	}
}