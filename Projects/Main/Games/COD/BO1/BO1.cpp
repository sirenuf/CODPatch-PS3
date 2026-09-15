#include "BO1.hpp"

namespace BO1
{
	namespace {
		std::set<std::string> buildGameIDsSet()
		{
			std::set<std::string> s;

			s.insert("NPEB00757");
			s.insert("NPEB00758");
			s.insert("BLKS20228");
			s.insert("BLJM60286");
			s.insert("NPJB90371");
			s.insert("BLES01034");
			s.insert("BLJM60537");
			s.insert("BLES01035");
			s.insert("BLUS30591F");
			s.insert("NPEB00756");
			s.insert("NPEB90323");
			s.insert("BLJM60536");
			s.insert("BLES01033");
			s.insert("BLJM61005");
			s.insert("BLES01105");
			s.insert("BLES01032");
			s.insert("NPUB30584");
			s.insert("BLJM61004");
			s.insert("NPHB00346");
			s.insert("BLES01031");
			s.insert("NPUB90430");
			s.insert("BLUS30638");
			s.insert("NPEB00760");
			s.insert("BLJM60287");
			s.insert("NPEB00759");
			s.insert("BLUS30591");
			s.insert("NPUB30604");

			return s;
		};
	}

	const std::set<std::string>& GetGameIDs()
	{
		static std::set<std::string> s = buildGameIDsSet();
		return s;
	}
}
