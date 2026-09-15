#include "Utils/Globals.hpp"

namespace Globals
{
	const char* GetIpcCacheFile()
	{
		return "/dev_hdd0/tmp/CODPATCH_IPC_CACHE.bin";
	}

	std::string GetRootDir()
	{
		return "/dev_hdd0/plugins/CODPatch";
	}

	std::string GetModulesDir()
	{
		return GetRootDir() + "/Modules";
	}
}