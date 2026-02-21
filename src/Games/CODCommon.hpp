#pragma once
#include <string>

namespace CODCommon
{
	enum CODType
	{
		MW2 = 0
	};

	// Unfortunately can't use std::string because of weird initialisation error.
	extern const char* CODTypeS[];

	struct MemoryLocation
	{
		size_t offset;
		size_t size;
	};

	bool VerifyFilesystem(CODType gameType);
	void SaveCurrentStats(CODType gameType);
	void LoadSavedStats(CODType gameType);

	/** For when game has connected to activision. You can then write to its stats without it getting overwritten.
	 * @return Boolean determining if game is ready or not.
	 */
	bool IsGameReady(CODType gameType);
}