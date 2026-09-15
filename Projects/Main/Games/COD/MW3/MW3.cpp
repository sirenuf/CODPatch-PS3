#include "MW3.hpp"

namespace MW3
{
	namespace
	{
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
}