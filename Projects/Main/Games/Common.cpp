#include "Common.hpp"

#include "Utils/SystemCalls.hpp"
#include "Games/FindActiveGame.hpp"

#include <libpsutil.h>

#define LIBFS_LOADED 0
#define EARLY_PID_FAILURE -1
#define GENERAL_FAILURE -2

namespace Common
{
	// This waits for the filesystem and indirectly lets us know 
	int CheckForFilesystem(u32 pid)
	{
		bool fsLibFound	 = false;
		bool netLibFound = false;

		const int PRX_ID_BUFFER_SIZE = 128;
		sys_prx_id_t prx_id_buffer[PRX_ID_BUFFER_SIZE]{};

		int result{};

		result = ps3mapi_get_all_process_modules_prx_id(pid, prx_id_buffer);	
		if (result < 0)
			return EARLY_PID_FAILURE;

		for (int i = 0; i < PRX_ID_BUFFER_SIZE; ++i)
		{
			char moduleName[128]{};
			result = ps3mapi_get_process_module_name_by_prx_id(pid, prx_id_buffer[i], moduleName);

			if (moduleName[0] == '\0')
				break;

			if (strncmp(moduleName, "sys_fs_Library", sizeof(moduleName)) == 0)
				fsLibFound = true;
			else if (strncmp(moduleName, "sys_net_Library", sizeof(moduleName)) == 0)
				netLibFound = true;

			// Consider leaving in for Debug.
			// printf("[WaitForFilesystem] moduleName is: %s\n", moduleName);
		}

		if (fsLibFound && netLibFound)
			return LIBFS_LOADED;
		
		return GENERAL_FAILURE;
	}

	int WaitForFilesystem(u32 pid)
	{
		int tries = 0;
		int result = GENERAL_FAILURE;

		while (tries < 10)
		{
			result = CheckForFilesystem(pid);

			if (result == LIBFS_LOADED)
				return LIBFS_LOADED;
			else if (result == EARLY_PID_FAILURE)
				return EARLY_PID_FAILURE;

			++tries;
			libpsutil::sleep(800);
		}

		return result;
	}

	void LoadSprx(Globals::Game game)
	{
		int waitResult = WaitForFilesystem(g_FindActiveGame.GetRunningGameProcessId());
		if (waitResult != 0) {
			printf("[CODPatch] Process dead, skipping...\n", waitResult);
			return;
		}
		
		printf("[CODPatch] Right candidate for injecting found.\n");

		bool results = libpsutil::filesystem::write_file(Globals::GetIpcCacheFile(), &game, sizeof(game));
		sys_fs_chmod(Globals::GetIpcCacheFile(), CELL_FS_S_IRUSR | CELL_FS_S_IWUSR | CELL_FS_S_IRGRP | CELL_FS_S_IROTH); // Set correct permissions on the file so game process can read it.

		std::string path = Globals::GetModulesDir() + "/Loader.sprx";
		printf("[CODPatch] Loading SPRX. Full path:\n\t↳ %s\n", path.data());

		// NOTE: I can't get argument pointer to work properly, I just get it pointing to garbage in the loader.
		i32 result = ps3mapi_load_process_modules(g_FindActiveGame.GetRunningGameProcessId(), (char*)path.data(), nullptr, 0);
		if (result != SUCCEEDED)
			printf("[CODPatch] Unexpected return code of ps3mapi_load_process_modules when loading Loader.sprx. Code: 0x%08X (int: %d)\n", (u32)result, result);
	}
}