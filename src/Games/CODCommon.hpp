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

	/**
	 * A structure which contains the memory address and how many bytes that memory address contains its data.
	 * @return
	 */
	struct MemoryEntry
	{
		MemoryEntry(size_t address, u16 size)
			: Address(address), Size(size) {
		}

		size_t Address;
		u16 Size;
	};

	/**
	 * A structure which is meant to contain MemoryEntry.
	 * name is only reserved for comments and identification.
	 * @return
	 */
	struct MemoryMapping
	{
		MemoryMapping(const char* name, MemoryEntry entry)
			: Name(name), Entry(entry) {
		}

		const char* Name;
		MemoryEntry Entry;
	};

	/**
	 * Verifies that the right folders and files exist and the integrity of the data save file.
	 * @returns
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
	 * @returns Boolean determining if game is ready or not.
	 */
	bool IsGameReady(CODType gameType);

	/**
	 * @returns The file path of the project.
	 */
	std::string GetRootDir();
	
	/**
	 * @returns The file path for storing the game stats data.
	 */
	std::string GetDataFilePath(CODType gameType);
}
