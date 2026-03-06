#include "CODCommon.hpp"

#include <vsh/vshtask.hpp>
#include <vsh/stdc.hpp>
#include <libpsutil.h>
#include <algorithm>

#include "Memory/Memory.hpp"
#include "Games/FindActiveGame.hpp"
#include "Games/COD/MW2.hpp"

#include "Utils/StringFunctions.hpp"

namespace CODCommon
{
	using namespace libpsutil::filesystem;
	using namespace StringFunctions;
	
	const char* CODTypeS[] = { "mw2", "mw3" };
	
	// If crashing in the future, make sure this isn't null :)
	xusers_t* GetXusers()
	{
		static xusers_t* xusers = xsetting_CC56EB2D();
		return xusers;
	}

	std::string GetRootDir()
	{
		return "/dev_hdd0/plugins/CODPatch";
	}

	std::string GetUserDir()
	{
		return GetRootDir() + "/data/" + to_string(GetXusers()->GetCurrentUserNumber());
	}

	std::string GetDataFilePath(CODType gameType)
	{
		return GetUserDir() + "/save_data_" + CODTypeS[gameType] + ".txt";
	}


	bool VerifyFilesystem(CODType gameType)
	{
		std::string DataPath = GetDataFilePath(gameType);

		create_directory(GetRootDir());
		create_directory(GetRootDir() + "/data");
		create_directory(GetUserDir());

		bool storageFileExists = file_exists(DataPath);

		// Make sure to remove CRs incase file has been edited on Windows!
		if (storageFileExists)
		{
			std::string s = read_file(DataPath);

			s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

			write_file(DataPath, s);
		}

		return storageFileExists;
	}


	bool IsGameReady(CODType gameType)
	{
		char buffer[8]{};
		
		// Sets class name in buffer.
		switch (gameType) {
			case MW2:
				ReadProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)0x01FF9E6C, buffer, sizeof(buffer));
				break;

			case MW3:
				ReadProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)0x1C1982C, buffer, sizeof(buffer));
				break;
		}

		for (int i = 0; i < sizeof(buffer); ++i)
			if (buffer[i] != NULL) // If class name has been retrieved, profile has init
				return true;

		return false;

	}


	void LoadSavedStats(CODType gameType)
	{
		// Fixed: https://github.com/skiff/libpsutil/pull/7
		std::string dataFile = read_file(GetDataFilePath(gameType));

		// position is the internal c string
		char* position = (char*)dataFile.c_str();

		// Check if file is empty
		char* newline = vsh::strchr(position, '\n');
		if (newline == nullptr)
			return;

		while (*(position) != '\0')
		{
			size_t address;
			std::vector<u8> secondItem;

			newline = vsh::strchr(position, '\n');

			// Skip comment and blank spaces.
			if (*(position) == '#' || *(position) == '\n') {
				position = newline + 1;
				continue;
			}

			std::string firstItemS(position, newline);

			address = ConvertStringToNumber(firstItemS);

			// Do next line	
			position = newline + 1;
			newline = vsh::strchr(position, '\n');

			if (newline == nullptr)
				newline = position + vsh::strlen(position); // point to end of string instead

			std::string secondItemS(position, newline);

			secondItem = ConvertStringToByteBuffer(secondItemS);

			position = newline + 1;
																						// ⬐ alternative to vector.data()
			WriteProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)address, &secondItem[0], secondItem.size());
		}
	}

	void SaveCurrentStats(CODType gameType)
	{
		u8 ByteBuffer[1];
		std::string saveString;

		const std::vector<MemoryMapping>* memory = nullptr;
		
		switch (gameType)
		{
			case MW2:
				memory = &MW2::GetMemory();
				break;

			default:
				vshtask::Notify("Attempt to get memory from unknown game.");
				return;
		}


		for (const MemoryMapping& map : *memory)
		{
			const std::string& name = map.Name;
			const MemoryEntry& entry = map.Entry;

			saveString += "# " + name + "\n";							// Comment
			saveString += ConvertIntToHexString(entry.Address) + "\n";	// Address

			// Get bytes
			for (int i = 0; i < entry.Size; ++i)
			{
				int result = ReadProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)(entry.Address + i), ByteBuffer, sizeof(ByteBuffer));
				if (result < 0)
					return;

				std::string byte = ConvertIntToHexString(*ByteBuffer);

				saveString += byte + " ";
			}

			saveString += "\n\n";
		}

		write_file(GetDataFilePath(gameType), saveString);
	}
};
