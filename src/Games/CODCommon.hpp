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

	/**
	 * Verifies that the right folders and files exist.
	 * @return
	 * Boolean determines if file exists or not.
	 */
	bool VerifyFilesystem(CODType gameType);
	
	void SaveCurrentStats(CODType gameType);

	/**
	 * Loads saved stats on filesystem.
	 * Intended to be ran on startup only.
	 * @return
	 */
	void LoadSavedStats(CODType gameType);

	/**
	 * For when game has connected to Activision. You can then write to its stats without it getting overwritten.
	 * @return Boolean determining if game is ready or not.
	 */
	bool IsGameReady(CODType gameType);
}