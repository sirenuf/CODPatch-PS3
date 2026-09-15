#pragma once
#include <string>

/**
 * Defined here rather than in a .cpp so that anything putting Projects\Main on its
 * include path gets them without that translation unit having to be added to its
 * project as well. inline is what makes it legal: every file including this header
 * emits a definition, and the linker keeps one rather than rejecting the duplicates.
 *
 * Note these have to stay functions - inline variables are C++17 and this builds as
 * C++11.
 */
namespace Globals
{
	enum Game {
		None,
		MW2,
		MW3,
		BO1,
		BO2
	};

	/**
	 * @returns The file path for IPC communication to the Loader.sprx
	 */
	const char* GetIpcCacheFile();

	/**
	 * @returns The directory path of the project.
	 */
	std::string GetRootDir();

	/**
	 * @returns The directory path of the modules.
	 */
	std::string GetModulesDir();
}
