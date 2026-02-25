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
	
	const char* CODTypeS[] = { "mw2" };
	
	
	std::string GetRootDir()
	{
		return "/dev_hdd0/tmp/CODPatch/";
	}

	
	std::string GetDataFilePath(CODType gameType)
	{
		return GetRootDir() + CODTypeS[gameType] + "/data";
	}


	bool VerifyFilesystem(CODType gameType)
	{

		std::string DataPath = GetDataFilePath(gameType);

		create_directory(GetRootDir());
		create_directory(GetRootDir() + CODTypeS[gameType]);

		bool storageFileExists;
		storageFileExists = file_exists(DataPath);

		// Make sure to remove CRs incase file has been edited on Windows!
		if (storageFileExists)
		{
			std::string s = read_file(DataPath);

			s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

			write_file(DataPath, s);
		}
		else
		{
			write_file(DataPath, "");
		}

		return storageFileExists;
	}


	bool IsGameReady(CODType gameType)
	{
		char buffer[8]{};
		if (gameType == MW2)													 // This is the address for the NAT type.
			ReadProcessMemory(g_FindActiveGame.GetRunningGameProcessId(), (void*)0x1C103AF, buffer, sizeof(buffer));

		// Need to convert to std::string
		std::string status = buffer;		

		// When "Your NAT Type" in the menu has a value
		// Then the game has connected.
		return 
			libpsutil::string::begins_with(status, "Open") ||
			libpsutil::string::begins_with(status, "Moderate") ||
			libpsutil::string::begins_with(status, "Strict");
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

		const std::hash_map<std::string, MemoryEntry>* memory = nullptr;
		
		switch (gameType)
		{
			case MW2:
				memory = &MW2::GetMemory();
				break;
		}


		for (const auto& pair : *memory)
		{
			const std::string& key = pair.first;
			const MemoryEntry& entry = pair.second;

			saveString += "# " + key + "\n";							// Comment
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
