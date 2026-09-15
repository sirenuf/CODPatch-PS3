#include "BO2.hpp"

namespace BO2
{
	namespace {
		std::set<std::string> buildGameIDsSet()
		{
			std::set<std::string> s;

			s.insert("NPUB31055");
			s.insert("BCKS10223");
			s.insert("BLUS41005SA");
			s.insert("BLJM60548");
			s.insert("BLUS31080");
			s.insert("BLUS41005F");
			s.insert("BLJM61231");
			s.insert("BLES01718");
			s.insert("BCUS91450");
			s.insert("NPUB31056");
			s.insert("NPEB01206");
			s.insert("BLJM61230");
			s.insert("NPEB01207");
			s.insert("BLUS31141SA");
			s.insert("BLUS31011");
			s.insert("BLJM61110");
			s.insert("BCKS10232");
			s.insert("NPEB01205");
			s.insert("NPEB01204");
			s.insert("BLES01719");
			s.insert("BLES01720");
			s.insert("BLUS31140");
			s.insert("BLJM61109");
			s.insert("BLJM60549");
			s.insert("BLES01717");
			s.insert("BLUS41005S");

			return s;
		};
	}

	const std::set<std::string>& GetGameIDs()
	{
		static std::set<std::string> s = buildGameIDsSet();
		return s;
	}
}