#pragma once
#include <string>

namespace Globals
{
	enum Game {
		None,
		MW2,
		MW3,
		BO1,
		BO2
	};

	/* @returns The file path for IPC communication to the Loader.sprx */
	const char* GetIpcCacheFile();

	/* @returns The directory path of the project. */
	std::string GetRootDir();

	/* @returns The directory path of the modules. */
	std::string GetModulesDir();
}
