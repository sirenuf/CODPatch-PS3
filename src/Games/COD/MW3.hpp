#pragma once
#include <set>

#include "Games/CODCommon.hpp"
#include "Games/FindActiveGame.hpp"

#include "Memory/Memory.hpp"

#include <libpsutil.h>

namespace MW3
{
	using CODCommon::MW3;

	const std::set<std::string>& GetGameIDs();

	void Run();
}