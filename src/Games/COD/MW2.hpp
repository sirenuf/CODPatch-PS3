#pragma once 
#include <vsh/stdc.hpp>
#include <vsh/vshtask.hpp>
#include <libpsutil.h>
#include <hash_map>
#include <set>

#include "Utils/Std.hpp"
#include "Memory/Memory.hpp"
#include "Games/CODCommon.hpp"

namespace MW2
{
	using CODCommon::MemoryMapping;

	const std::set<std::string>& GetGameIDs();
	const std::vector<MemoryMapping>& GetMemory();

	void Run();
}