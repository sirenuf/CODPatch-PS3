#pragma once
#include <string>

#include "Utils/Types.hpp"

namespace CODCommon
{

	enum CODType
	{
		MW2 = 0
	};

	// Unfortunately can't use std::string because of weird initialisation error.
	extern const char* CODTypeS[];

	struct MemoryEntry
	{
		size_t Address;
		u16 Size; // Amount of bytes to read at address.
	};

	/**
	 * Verifies that the right folders and files exist and the integrity of the data save file.
	 * @return
	 * Boolean determines if the data file existed or not.
	 */
	bool VerifyFilesystem(CODType gameType);
	
	void SaveCurrentStats(CODType gameType);

	/**
	 * Loads saved stats saved on filesystem.
	 * Intended to be ran on startup only.
	 * @return
	 */
	void LoadSavedStats(CODType gameType);

	/**
	 * For when game has connected to Activision. You can then write to its stats without it getting overwritten.
	 * @return Boolean determining if game is ready or not.
	 */
	bool IsGameReady(CODType gameType);

	/**
	 * @return The file path of the project.
	 */
	std::string GetRootDir();
	
	/**
	 * @return The file path for storing the game stats data.
	 */
	std::string GetDataFilePath(CODType gameType);
}
